//
//  render.cpp
//  FrameTech
//
//  Created by Antonin on 28/09/2022.
//

#include "render.hpp"
#include "../../application.hpp"
#include "../../ftstd/debug_tools.h"
#include "../../ftstd/result.h"
#include "../../project.hpp"
#include "../engine.hpp"
#include <vector>

// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

FrameTech::Graphics::Render* FrameTech::Graphics::Render::m_instance{nullptr};

FrameTech::Graphics::Render::Render()
{
    m_graphics_pipeline = std::unique_ptr<FrameTech::Graphics::Pipeline>(new FrameTech::Graphics::Pipeline());
}

FrameTech::Graphics::Render::~Render()
{
    if (m_graphics_pipeline != nullptr)
    {
        Log("< Destroying the graphics pipeline...");
        m_graphics_pipeline = nullptr;
    }
    if (m_surface != VK_NULL_HANDLE)
    {
        Log("< Destroying the window surface...");
        vkDestroySurfaceKHR(FrameTech::Engine::getInstance()->m_graphics_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_image_views.size() > 0)
    {
        Log("< Destroying the image views...");
        for (auto image_view : m_image_views)
            vkDestroyImageView(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(), image_view, nullptr);
        m_image_views.clear();
    }
    m_instance = nullptr;
}

FrameTech::Graphics::Render* FrameTech::Graphics::Render::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Render();
    }
    return m_instance;
}

VResult FrameTech::Graphics::Render::createSurface()
{
    const auto window_surface_result = glfwCreateWindowSurface(
        FrameTech::Engine::getInstance()->m_graphics_instance,
        FrameTech::Application::getInstance(Project::APPLICATION_NAME)->getWindow(),
        nullptr,
        &m_surface);
    if (window_surface_result == VK_SUCCESS)
    {
        return VResult::Ok();
    }
    return VResult::Error((char*)"failed to create a window surface");
}

VkSurfaceKHR* FrameTech::Graphics::Render::getSurface()
{
    return &m_surface;
}

VResult FrameTech::Graphics::Render::createImageViews()
{
    const auto swapchain_images = FrameTech::Engine::getInstance()->m_swapchain->getImages();
    const size_t nb_swapchain_images = swapchain_images.size();
    m_image_views.resize(nb_swapchain_images);
    Log("> %d image views to create (for the render object)", nb_swapchain_images);
    for (size_t i = 0; i < nb_swapchain_images; i++)
    {
        // Create a VkImageView for each VkImage from the swapchain
        VkImageViewCreateInfo image_view_create_info{

            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain_images[i],
            // How the image data should be interpreted
            .viewType = VK_IMAGE_VIEW_TYPE_2D, // could be 1D / 2D / 3D texture, or cube maps
            .format = FrameTech::Engine::getInstance()->m_swapchain->getImageFormat().format,
            // Default mapping
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };
        const auto image_view_result = vkCreateImageView(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
                                                         &image_view_create_info,
                                                         nullptr,
                                                         &m_image_views[i]);
        if (image_view_result == VK_SUCCESS)
        {
            Log("\t* image view %d... ok!", i);
            continue;
        }

        char* error_msg;
        switch (image_view_result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                error_msg = (char*)"out of host memory";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                error_msg = (char*)"out of device memory";
                break;
            default:
                LogE("> vkCreateImageView: unknown error 0x%08x", image_view_result);
                error_msg = (char*)"undocumented error";
                break;
        }
        LogE("Error creating the image view %d: %s", i, error_msg);
        return VResult::Error(error_msg);
    }
    return VResult::Ok();
}

VResult FrameTech::Graphics::Render::createGraphicsPipeline()
{
    // TODO: vector of ShaderModule ?
    const Result<std::vector<FrameTech::Graphics::Shader::Module>> shaders_compile_result = m_graphics_pipeline->createGraphicsApplication(
        "shaders/basic_triangle.vert.spv",
        "shaders/basic_triangle.frag.spv");
    if (shaders_compile_result.IsError())
        return VResult::Error((char*)"cannot compile the application shaders");
    const std::vector<FrameTech::Graphics::Shader::Module> shaders_compiled = shaders_compile_result.GetValue();
    if (shaders_compiled.size() == 0)
    {
        LogW("No compiled shaders - check if alright");
        VResult::Ok();
    }
    int shader_index = 0;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages(shaders_compiled.size());
    for (const auto c_shader : shaders_compiled)
    {
        Log("> Creating shader module for %s (size of %d bytes), with type %d", c_shader.m_tag, c_shader.m_size, c_shader.m_type);
        VkShaderModuleCreateInfo shader_module_create_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = c_shader.m_size,
            .pCode = reinterpret_cast<const uint32_t*>(c_shader.m_code)};

        // Create the shader module in order to create the stage right after
        VkShaderModule shader_module;
        const auto shader_module_creation_result = vkCreateShaderModule(
            FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
            &shader_module_create_info,
            nullptr,
            &shader_module);
        if (shader_module_creation_result != VK_SUCCESS)
        {
            char* error_msg;
            switch (shader_module_creation_result)
            {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    error_msg = (char*)"out of host memory";
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    error_msg = (char*)"out of device memory";
                    break;
                case VK_ERROR_INVALID_SHADER_NV:
                    error_msg = (char*)"invalid shader";
                    break;
                default:
                    error_msg = (char*)"undocumented error";
                    break;
            }
            LogE("< Error creation the shader module: %s", error_msg);
            return VResult::Error(error_msg);
        }

        // TODO: create the shader stages
        WARN_RT_UNIMPLEMENTED;
        VkPipelineShaderStageCreateInfo shader_stage_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pName = c_shader.m_entrypoint,
            .module = shader_module,
        };
        switch (c_shader.m_type)
        {
            case Shader::COMPUTE_SHADER:
                shader_stage_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                break;
            case Shader::FRAGMENT_SHADER:
                shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case Shader::GEOMETRY_SHADER:
                shader_stage_create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                break;
            case Shader::VERTEX_SHADER:
                shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            default:
                LogW("> Shader with internal type %d is not managed", c_shader.m_type);
                WARN_RT_UNIMPLEMENTED;
                break;
        }

        shader_stages[shader_index++] = shader_stage_create_info;

        WARN_RT_UNIMPLEMENTED;

        // Don't need to store or use the shader module later
        vkDestroyShaderModule(
            FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
            shader_module,
            nullptr);
    }
    // Should not happen
    if (shader_stages.size() <= 0)
    {
        WARN_RT_UNIMPLEMENTED;
        LogW("> There is %d shader stages, instead of %d", shader_stages.size(), shaders_compiled.size());
        return VResult::Error((char*)"cannot set NULL shader stages");
    }
    m_graphics_pipeline->setShaderStages(shader_stages);
    return VResult::Ok();
}
