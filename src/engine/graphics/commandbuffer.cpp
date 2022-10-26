//
//  pool.cpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#include "commandbuffer.hpp"
#include "../engine.hpp"

FrameTech::Graphics::CommandBuffer* FrameTech::Graphics::CommandBuffer::m_instance{nullptr};

FrameTech::Graphics::CommandBuffer* FrameTech::Graphics::CommandBuffer::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new CommandBuffer();
    }
    return m_instance;
};

FrameTech::Graphics::CommandBuffer::CommandBuffer(){};

FrameTech::Graphics::CommandBuffer::~CommandBuffer()
{
    if (m_pool != nullptr)
    {
        vkDestroyCommandPool(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(), (*m_pool), nullptr);
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
        m_pool);
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
    alloc_info.commandPool = (*m_pool);
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    const auto create_result = vkAllocateCommandBuffers(
        FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &alloc_info,
        m_buffer);
    if (create_result == VK_SUCCESS)
        return VResult::Ok();
    return VResult::Error((char*)"> Error creating the buffer in the command buffer object");
}

VResult FrameTech::Graphics::CommandBuffer::record(VkCommandBuffer command_buffer, uint32_t swapchain_index)
{
    WARN_CT_UNIMPLEMENTED;
    return VResult::Error((char*)"> Error recording the command buffer");
}

VkCommandBuffer* FrameTech::Graphics::CommandBuffer::getBuffer()
{
    return m_buffer;
};

VkCommandPool* FrameTech::Graphics::CommandBuffer::getPool()
{
    return m_pool;
};
