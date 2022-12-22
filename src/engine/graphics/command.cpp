//
//  command.cpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#include "command.hpp"
#include "../engine.hpp"

#ifdef IMGUI
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#endif

frametech::graphics::Command::Command(){};

frametech::graphics::Command::~Command()
{
    if (m_pool != nullptr)
    {
        vkDestroyCommandPool(frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(), m_pool, nullptr);
        m_pool = nullptr;
    }
    m_buffer = nullptr;
};

ftstd::VResult frametech::graphics::Command::createPool(const uint32_t family_index)
{
    VkCommandPoolCreateInfo pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = family_index,
    };
    const auto create_result = vkCreateCommandPool(
        frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &pool_create_info,
        nullptr,
        &m_pool);
    if (create_result == VK_SUCCESS)
        return ftstd::VResult::Ok();
    return ftstd::VResult::Error((char*)"> Error creating the command pool in the command buffer object");
}

ftstd::VResult frametech::graphics::Command::createBuffer()
{
    if (m_pool == nullptr)
        return ftstd::VResult::Error((char*)"> Error creating the buffer: no memory pool");
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = (m_pool);
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    const auto create_result = vkAllocateCommandBuffers(
        frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &alloc_info,
        &m_buffer);
    if (create_result == VK_SUCCESS)
        return ftstd::VResult::Ok();
    return ftstd::VResult::Error((char*)"> Error creating the buffer in the command buffer object");
}

ftstd::VResult frametech::graphics::Command::record()
{
    const auto swapchain_index = frametech::Engine::getInstance()->m_render->getFrameIndex();
    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    // Reset the command buffer before any operation on the current buffer
    vkResetCommandBuffer(m_buffer, 0);

    if (const auto begin_result_code = vkBeginCommandBuffer(m_buffer, &begin_info); begin_result_code != VK_SUCCESS)
    {
        return ftstd::VResult::Error((char*)"< Error creating the command buffer");
    }

    const std::vector<VkFramebuffer> framebuffers = frametech::Engine::getInstance()->m_render->getFramebuffers();
    if (swapchain_index >= framebuffers.size())
    {
        return ftstd::VResult::Error((char*)"< The swapchain_index parameter is incorrect: not enough framebuffers");
    }

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getRenderPass(),
        .framebuffer = framebuffers[swapchain_index],
        .renderArea = {
            .offset = {0, 0},
            .extent = frametech::Engine::getInstance()->m_swapchain->getExtent(),
        },
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(m_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        m_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getPipeline());

    // Setup the viewport and scissor as dynamic
    // TODO: fix this in the fixed function
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(frametech::Engine::getInstance()->m_swapchain->getExtent().width),
        .height = static_cast<float>(frametech::Engine::getInstance()->m_swapchain->getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(m_buffer, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = frametech::Engine::getInstance()->m_swapchain->getExtent(),
    };
    vkCmdSetScissor(m_buffer, 0, 1, &scissor);

    // Bind the vertex buffer
    std::vector<VkBuffer> vertex_buffers = {frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getVertexBuffer()};
    // // TODO: check to include this information getting the vertex buffer
    std::vector<VkDeviceSize> memory_offsets(vertex_buffers.size());
    const VkBuffer& index_buffer = frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getIndexBuffer();
    for (size_t i = 0; i < vertex_buffers.size(); ++i)
        memory_offsets[i] = i;
    vkCmdBindVertexBuffers(m_buffer, 0, vertex_buffers.size(), vertex_buffers.data(), memory_offsets.data());
    vkCmdBindIndexBuffer(m_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    uint32_t indices_size = static_cast<uint32_t>(frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getIndices().size());

    vkCmdDrawIndexed(m_buffer, indices_size, 1, 0, 0, 0);

#ifdef IMGUI
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, m_buffer);
#endif

    vkCmdEndRenderPass(m_buffer);
    if (const auto end_command_buffer_result_code = vkEndCommandBuffer(m_buffer); end_command_buffer_result_code != VK_SUCCESS)
    {
        return ftstd::VResult::Error((char*)"< Error recording the command buffer");
    }

    return ftstd::VResult::Ok();
}

VkCommandBuffer* frametech::graphics::Command::getBuffer()
{
    return &m_buffer;
};

VkCommandPool* frametech::graphics::Command::getPool()
{
    return &m_pool;
};
