//
//  pool.cpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#include "commandbuffer.hpp"
#include "../engine.hpp"

FrameTech::Graphics::CommandBuffer::CommandBuffer(){};

FrameTech::Graphics::CommandBuffer::~CommandBuffer()
{
    if (m_pool != nullptr)
    {
        vkDestroyCommandPool(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(), m_pool, nullptr);
        m_pool = nullptr;
    }
    m_buffer = nullptr;
};

VResult FrameTech::Graphics::CommandBuffer::createPool()
{
    VkCommandPoolCreateInfo pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = FrameTech::Engine::getInstance()->m_graphics_device.m_graphics_queue_family_index,
    };
    const auto create_result = vkCreateCommandPool(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &pool_create_info,
        nullptr,
        &m_pool);
    if (create_result == VK_SUCCESS)
        return VResult::Ok();
    return VResult::Error((char*)"> Error creating the command pool in the command buffer object");
}

VResult FrameTech::Graphics::CommandBuffer::createBuffer()
{
    if (m_pool == nullptr)
        return VResult::Error((char*)"> Error creating the buffer: no memory pool");
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = (m_pool);
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    const auto create_result = vkAllocateCommandBuffers(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &alloc_info,
        &m_buffer);
    if (create_result == VK_SUCCESS)
        return VResult::Ok();
    return VResult::Error((char*)"> Error creating the buffer in the command buffer object");
}

VResult FrameTech::Graphics::CommandBuffer::record()
{
    const auto swapchain_index = FrameTech::Engine::getInstance()->m_render->getFrameIndex();
    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    if (const auto begin_result_code = vkBeginCommandBuffer(m_buffer, &begin_info); begin_result_code != VK_SUCCESS)
    {
        return VResult::Error((char*)"< Error creating the command buffer");
    }

    const std::vector<VkFramebuffer> framebuffers = FrameTech::Engine::getInstance()->m_render->getFramebuffers();
    if (swapchain_index >= framebuffers.size())
    {
        return VResult::Error((char*)"< The swapchain_index parameter is incorrect: not enough framebuffers");
    }

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = FrameTech::Engine::getInstance()->m_pipeline.getRenderPass(),
        .framebuffer = framebuffers[swapchain_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = FrameTech::Engine::getInstance()->m_swapchain->getExtent(),
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(m_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        m_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        FrameTech::Engine::getInstance()->m_pipeline.getPipeline());

    // Setup the viewport and scissor as dynamic
    // TODO: fix this in the fixed function
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(FrameTech::Engine::getInstance()->m_swapchain->getExtent().width),
        .height = static_cast<float>(FrameTech::Engine::getInstance()->m_swapchain->getExtent().height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(m_buffer, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = FrameTech::Engine::getInstance()->m_swapchain->getExtent(),
    };
    vkCmdSetScissor(m_buffer, 0, 1, &scissor);

    // TODO: change to dynamic draw command
    vkCmdDraw(m_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_buffer);
    if (const auto end_command_buffer_result_code = vkEndCommandBuffer(m_buffer); end_command_buffer_result_code != VK_SUCCESS)
    {
        return VResult::Error((char*)"< Error recording the command buffer");
    }

    return VResult::Ok();
}

VkCommandBuffer* FrameTech::Graphics::CommandBuffer::getBuffer()
{
    return &m_buffer;
};

VkCommandPool* FrameTech::Graphics::CommandBuffer::getPool()
{
    return &m_pool;
};
