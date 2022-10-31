//
//  pipeline.cpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#include "pipeline.hpp"
#include "../../ftstd/debug_tools.h"
#include "../../ftstd/result.h"
#include "../engine.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

FrameTech::Graphics::Pipeline::Pipeline()
{
}

FrameTech::Graphics::Pipeline::~Pipeline()
{
    const auto graphics_device = FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice();
    if (m_shader_modules.size() > 0)
    {
        Log("< Destroying the shader modules...");
        for (const auto shader_module : m_shader_modules)
            vkDestroyShaderModule(
                graphics_device,
                shader_module,
                nullptr);
    }
    if (m_render_pass != VK_NULL_HANDLE)
    {
        Log("< Destroying the render pass...");
        vkDestroyRenderPass(graphics_device, m_render_pass, nullptr);
        m_render_pass = VK_NULL_HANDLE;
    }
    if (m_layout != VK_NULL_HANDLE)
    {
        Log("< Destroying the pipeline layout...");
        vkDestroyPipelineLayout(graphics_device, m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }
    if (m_pipeline != VK_NULL_HANDLE)
    {
        Log("< Destroying the pipeline object...");
        vkDestroyPipeline(graphics_device, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
    if (nullptr != m_sync_image_ready)
    {
        Log("< Destroying the image ready signal semaphore...");
        vkDestroySemaphore(graphics_device, *m_sync_image_ready, nullptr);
        m_sync_image_ready = nullptr;
    }
    if (nullptr != m_sync_present_done)
    {
        Log("< Destroying the present done signal semaphore...");
        vkDestroySemaphore(graphics_device, *m_sync_present_done, nullptr);
        m_sync_present_done = nullptr;
    }
    if (nullptr != m_sync_cpu_gpu)
    {
        Log("< Destroying the fence...");
        vkDestroyFence(graphics_device, *m_sync_cpu_gpu, nullptr);
        m_sync_cpu_gpu = nullptr;
    }
}

std::optional<uint64_t> FrameTech::Graphics::Pipeline::fileSize(const char* filepath)
{
    try
    {
        const auto file_size = std::filesystem::file_size(filepath);
        return file_size;
    }
    catch (const std::exception& e)
    {
        LogE("< Error getting the file size at path '%s'", filepath);
        return std::nullopt;
    }
}

std::optional<uint64_t> FrameTech::Graphics::Pipeline::readFile(const char* filepath,
                                                                char** buffer,
                                                                uint64_t buffer_length)
{
    std::ifstream file(filepath, std::ifstream::binary | std::ifstream::ate);
    assert(file);
    if (file)
    {
        std::optional<uint64_t> opt_length = fileSize(filepath);
        if (opt_length == std::nullopt)
        {
            return std::nullopt;
        }
        uint64_t length = opt_length.value();

        // do not read more than enough
        length = buffer_length < length ? (int)buffer_length : length;

        // read data as a block, close, and return
        // the length of the file
        file.seekg(0);
        file.read(*buffer, length);
        file.close();
        return length;
    }
    return std::nullopt;
}

Result<std::vector<FrameTech::Graphics::Shader::Module>> FrameTech::Graphics::Pipeline::createGraphicsApplication(const char* vertex_shader_filepath,
                                                                                                                  const char* fragment_shader_filepath)
{
    // Get the length
    const auto vs_file_size_opt = fileSize(vertex_shader_filepath);
    assert(vs_file_size_opt != std::nullopt);
    const auto fs_file_size_opt = fileSize(fragment_shader_filepath);
    assert(fs_file_size_opt != std::nullopt);
    // If one of them are empty, fail
    if (vs_file_size_opt == std::nullopt || fs_file_size_opt == std::nullopt)
    {
        LogE("< Cannot create the program");
        return Result<std::vector<FrameTech::Graphics::Shader::Module>>::Error((char*)"vertex or fragment shader is NULL");
    }
    // Get the content of the VS
    const auto vs_file_size = vs_file_size_opt.value();
    auto vs_buffer = new char[vs_file_size];
    readFile(vertex_shader_filepath, &vs_buffer, vs_file_size);
    Log("> For VS file '%s', read file ok (%d bytes)", vertex_shader_filepath, vs_file_size);
    // Get the content of the FS
    const auto fs_file_size = fs_file_size_opt.value();
    auto fs_buffer = new char[fs_file_size];
    readFile(fragment_shader_filepath, &fs_buffer, fs_file_size);
    Log("> For FS file '%s', read file ok (%d bytes)", fragment_shader_filepath, fs_file_size);
    std::vector<FrameTech::Graphics::Shader::Module> shader_modules(
        {FrameTech::Graphics::Shader::Module{
             .m_tag = (char*)fragment_shader_filepath,
             .m_code = fs_buffer,
             .m_size = fs_file_size,
             .m_type = FrameTech::Graphics::Shader::FRAGMENT_SHADER,
         },
         FrameTech::Graphics::Shader::Module{
             .m_tag = (char*)vertex_shader_filepath,
             .m_code = vs_buffer,
             .m_size = vs_file_size,
             .m_type = FrameTech::Graphics::Shader::VERTEX_SHADER,
         }});
    return Result<std::vector<FrameTech::Graphics::Shader::Module>>::Ok(shader_modules);
}

void FrameTech::Graphics::Pipeline::setShaderModules(const std::vector<VkShaderModule> shader_modules)
{
    m_shader_modules = shader_modules;
}

void FrameTech::Graphics::Pipeline::setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo> stages)
{
    m_shader_stages = stages;
}

static VkViewport createViewport(size_t height, size_t width)
{
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.height = (float)height;
    viewport.width = (float)width;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

static VkRect2D createScissor(const VkExtent2D& swapchain_extent)
{
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain_extent;
    return scissor;
}

VkRenderPass& FrameTech::Graphics::Pipeline::getRenderPass()
{
    return m_render_pass;
}

VResult FrameTech::Graphics::Pipeline::setupRenderPass()
{
    Log("> Setting up the render pass object of the graphics pipeline");

    const auto NB_ATTACHMENTS = 1;
    // Setup the color attachment format & samples
    VkAttachmentDescription attachments[NB_ATTACHMENTS] = {
        VkAttachmentDescription{
            .format = FrameTech::Engine::getInstance()->m_swapchain->getImageFormat().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,        // No multi-sampling: 1 sample
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,   // Before rendering: clear the framebuffer to black before drawing
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE, // After rendering: store in memory to read it again later
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,     // Don't care what previous layout the image was in
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Images will be transitioned to the SwapChain for presentation
        },
    };

    // Subpasses and attachment references, as a render pass
    // can consist of multiple subpasses
    VkAttachmentReference color_attachment_reference{};
    color_attachment_reference.attachment = 0; // Index 0
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    const auto NB_SUBPASSES = 1;
    VkSubpassDescription subpasses[NB_SUBPASSES] = {
        VkSubpassDescription{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_reference,
        },
    };

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = NB_ATTACHMENTS;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = NB_SUBPASSES;
    render_pass_info.pSubpasses = subpasses;

    const auto create_result_code = vkCreateRenderPass(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &render_pass_info,
        nullptr,
        &m_render_pass);

    if (create_result_code != VK_SUCCESS)
    {
        return VResult::Error((char*)"Failed to create the render pass");
    }
    return VResult::Ok();
}

VResult FrameTech::Graphics::Pipeline::preconfigure()
{
    Log("> Preconfiguring the graphics pipeline");

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    const auto create_result_code = vkCreatePipelineLayout(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &pipeline_layout_create_info,
        nullptr,
        &m_layout);

    if (create_result_code != VK_SUCCESS)
    {
        return VResult::Error((char*)"Failed to create the pipeline layout!");
    }

    return createSyncObjects();
}

VResult FrameTech::Graphics::Pipeline::create()
{
    Log("> Creating the graphics pipeline");
    if (m_shader_stages.size() == 0)
    {
        return VResult::Error((char*)"No shader stages to finalize the graphics pipeline creation - ok?");
    }

    if (m_layout == VK_NULL_HANDLE)
    {
        return VResult::Error((char*)"Cannot create the graphics pipeline without pipeline layout information");
    }

    // TODO: make this array as a class parameter
    // If array belongs to the class parameter, move it to std::vector
    VkDynamicState dynamic_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = sizeof(dynamic_states) / sizeof(VkDynamicState),
        .pDynamicStates = dynamic_states,
    };

    // TODO: change for dynamic state, in order to pass the viewport & scissor
    // through the command buffer
    const VkExtent2D& swapchain_extent = FrameTech::Engine::getInstance()->m_swapchain->getExtent();
    const VkViewport viewport = createViewport(swapchain_extent.height, swapchain_extent.width);
    const VkRect2D scissor = createScissor(swapchain_extent);
    VkPipelineViewportStateCreateInfo viewport_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    // Vertex data settings:
    // * bindings: spacing between data, and whether the data is per-vertex or per-instance,
    // * attribute descriptions: type of the attributes passed to the vertex shader, offset, binding(s) to load, ...
    // No data for now as we are testing with vertex data in the shader directly
    // TODO: to change to pass it through the renderer
    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    // Describes the kind of geometry that will be used, and if primitive restart
    // should be enabled
    VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        // Setting VK_TRUE to primitiveRestartEnable returns a validation
        // error (VUID-VkPipelineInputAssemblyStateCreateInfo-topology-00428)
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };
    // rasterizer_state_create_info.depthBiasConstantFactor = 0.0f;
    // rasterizer_state_create_info.depthBiasClamp = 0.0f;
    // rasterizer_state_create_info.depthBiasSlopeFactor = 0.0f;

    // TODO: Check if needs another GPU feature
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    // TODO: Check if needed
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthBoundsTestEnable = VK_FALSE,
        .depthTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    // Combine fragment shader's color with existing framebuffer's color,
    // configured **per** framebuffer.
    // Both modes are disabled, the fragment colors will be written to
    // the framebuffer unmodified.
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
    };

    VkGraphicsPipelineCreateInfo pipeline_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = (uint32_t)m_shader_stages.size(),
        .pStages = m_shader_stages.data(),
        .pVertexInputState = &vertex_input_create_info,
        .pInputAssemblyState = &assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterizer_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = m_layout,
        .renderPass = m_render_pass,
        .subpass = 0, // index of the subpass
    };

    const auto create_result_code = vkCreateGraphicsPipelines(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        VK_NULL_HANDLE,
        1,
        &pipeline_info,
        nullptr,
        &m_pipeline);

    if (create_result_code != VK_SUCCESS)
    {
        return VResult::Error((char*)"Failed to create the main graphics pipeline");
    }
    return VResult::Ok();
}

VkPipeline FrameTech::Graphics::Pipeline::getPipeline()
{
    return m_pipeline;
}

VResult FrameTech::Graphics::Pipeline::createSyncObjects()
{
    Log("> Creating the sync objects");
    auto graphics_device = FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice();
    if (nullptr != m_sync_image_ready)
    {
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (VK_SUCCESS != vkCreateSemaphore(graphics_device, &create_info, nullptr, m_sync_image_ready))
            return VResult::Error((char*)"< Failed to create the semaphore to signal image ready");
    }
    if (nullptr != m_sync_image_ready)
    {
        VkSemaphoreCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (VK_SUCCESS != vkCreateSemaphore(graphics_device, &create_info, nullptr, m_sync_present_done))
            return VResult::Error((char*)"< Failed to create the semaphore to signal present is done");
    }
    if (nullptr != m_sync_image_ready)
    {
        VkFenceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // This allows to not wait for the first wait
        if (VK_SUCCESS != vkCreateFence(graphics_device, &create_info, nullptr, m_sync_cpu_gpu))
            return VResult::Error((char*)"< Failed to create the fence");
    }
    return VResult::Ok();
}

void FrameTech::Graphics::Pipeline::present()
{
}

Result<int> FrameTech::Graphics::Pipeline::draw()
{
    // Wait that all fences are sync...
    vkWaitForFences(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        1,
        m_sync_cpu_gpu,
        VK_TRUE,
        UINT64_MAX);
    // ... and reset them
    vkResetFences(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        1,
        m_sync_cpu_gpu);
    return Result<int>::Error((char*)"Draw function threw an error");
}
