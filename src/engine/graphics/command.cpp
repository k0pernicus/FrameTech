//
//  command.cpp
//  FrameTech
//
//  Created by Antonin on 26/10/2022.
//

#include "command.hpp"
#include "../engine.hpp"
#include "../../ftstd/profile_tools.h"

#ifdef IMGUI
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#endif

frametech::graphics::Command::Command(){};

frametech::graphics::Command::Command(VkCommandPool command_pool)
{
    m_pool = command_pool;
};

frametech::graphics::Command::~Command()
{
    m_pool = nullptr;
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
    if (VK_SUCCESS == create_result)
        return ftstd::VResult::Ok();
    m_queue_family_index_created_with = family_index;
    return ftstd::VResult::Error((char*)"> Error creating the command pool in the command buffer object");
}

ftstd::VResult frametech::graphics::Command::createBuffer()
{
    if (nullptr == m_pool)
        return ftstd::VResult::Error((char*)"> Error creating the buffer: no memory pool");
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    const auto create_result = vkAllocateCommandBuffers(
        frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        &alloc_info,
        &m_buffer);
    if (VK_SUCCESS != create_result)
        return ftstd::VResult::Error((char*)"> Error creating the buffer in the command buffer object");
    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::graphics::Command::begin()
{
    ftstd::profile::ScopedProfileMarker scope((char*)"frametech::graphics::Command::begin");
    assert(CommandState::S_ENDED == m_state || CommandState::S_UNKNOWN == m_state);
    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // Wait before submit
    };
    if (VK_SUCCESS == vkBeginCommandBuffer(m_buffer, &command_buffer_begin_info))
    {
        m_state = CommandState::S_BEGAN;
        return ftstd::VResult::Ok();
    }
    m_state = CommandState::S_ERROR;
    return ftstd::VResult::Error((char*)"> Error calling vkBeginCommandBuffer");
}

ftstd::VResult frametech::graphics::Command::end(const VkQueue& queue, const uint32_t submit_count)
{
    ftstd::profile::ScopedProfileMarker scope((char*)"frametech::graphics::Command::end");
    assert(CommandState::S_BEGAN == m_state);
    vkEndCommandBuffer(m_buffer);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_buffer,
    };

    vkQueueSubmit(queue, submit_count, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(
        frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
        m_pool,
        1,
        &m_buffer);

    m_state = CommandState::S_ENDED;
    return ftstd::VResult::Ok();
}

void frametech::graphics::Command::transition(
    const VkImage& image,
    const VkImageLayout new_layout,
    const VkImageLayout old_layout,
    const uint32_t src_queue_family_index,
    const uint32_t dst_queue_family_index) const noexcept
{
    assert(CommandState::S_BEGAN == m_state);

    VkAccessFlags src_access_mask;
    VkAccessFlags dst_access_mask;
    VkPipelineStageFlags src_stage_mask;
    VkPipelineStageFlags dst_stage_mask;

    // Compute the stage masks based on the layouts & index parameters
    switch (new_layout)
    {
        // Transition to copy the buffer to the image -> no need to wait
        // The transition **must** be occured in the Transfer stage
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        {
            src_access_mask = 0;
            dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        break;
        // Transition for a (fragment) shader -> wait on resource
        // Fragment shader as the image is considered as a texture
        // The resource is coming from the Transfer stage as it just
        // been copied from a buffer
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        {
            src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
            src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        break;
        default:
            // Let it crash, let it craaaaaaash...
            LogE("Error in transition : no right state found (new layout with %d)", new_layout);
            return;
    }

    VkImageMemoryBarrier memory_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_access_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = src_queue_family_index,
        .dstQueueFamilyIndex = dst_queue_family_index,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    vkCmdPipelineBarrier(m_buffer,
                         src_stage_mask, dst_stage_mask,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &memory_barrier);
}

ftstd::VResult frametech::graphics::Command::record()
{
    ftstd::profile::ScopedProfileMarker scope((char*)"frametech::graphics::Command::record");
    const auto current_frame_index = frametech::Engine::getInstance()->m_render->getFrameIndex();
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
    if (current_frame_index >= framebuffers.size())
    {
        return ftstd::VResult::Error((char*)"< The current_frame_index parameter is incorrect: not enough framebuffers");
    }
    
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0] = {{{0.0, 0.0, 0.0, 1.0}}}; // COLOR
    clear_values[1] = {{{1.0, 0}}};               // DEPTH

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getRenderPass(),
        .framebuffer = framebuffers[current_frame_index],
        .renderArea = {
            .offset = {0, 0},
            .extent = frametech::Engine::getInstance()->m_swapchain->getExtent(),
        },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(m_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        m_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getPipeline());

    // Setup the viewport and scissor as dynamic
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
    vkCmdBindVertexBuffers(m_buffer, 0, (uint32_t)vertex_buffers.size(), vertex_buffers.data(), memory_offsets.data());
    vkCmdBindIndexBuffer(m_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    // Bind the right descriptor set to the descriptors in the shaders
    std::optional<VkDescriptorSet*> current_descriptor_set = frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getDescriptorSet(current_frame_index);
    if (std::nullopt != current_descriptor_set)
    {
        vkCmdBindDescriptorSets(
            m_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getPipelineLayout(),
            0,
            1,
            current_descriptor_set.value(),
            0,
            nullptr);
    }

    uint32_t indices_size = static_cast<uint32_t>(frametech::Engine::getInstance()->m_render->getGraphicsPipeline()->getIndices().size());
    vkCmdDrawIndexed(m_buffer, indices_size, 1, 0, 0, 0);

#ifdef IMGUI
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, m_buffer);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Update and Render additional Platform Windows
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
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
