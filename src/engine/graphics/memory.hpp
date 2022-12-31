//
//  memory.hpp
//  FrameTech
//
//  Created by Antonin on 14/12/2022.
//

#pragma once
#ifndef memory_h
#define memory_h

#include "../../ftstd/result.h"
#include <assert.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        class Memory
        {
        private:
            static ftstd::Result<uint32_t> findMemoryType(
                const VkPhysicalDevice& physical_device,
                const uint32_t type_filter,
                const VkMemoryPropertyFlags memory_property_flags) noexcept
            {
                VkPhysicalDeviceMemoryProperties memory_properties{};
                vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
                for (uint32_t index = 0; index < memory_properties.memoryTypeCount; ++index)
                {
                    if (type_filter & (1 << index) && (memory_properties.memoryTypes[index].propertyFlags & memory_property_flags) == memory_property_flags)
                    {
                        return ftstd::Result<uint32_t>::Ok(index);
                    }
                }
                return ftstd::Result<uint32_t>::Error((char*)"findMemoryType: did not found any memory type with favorite filter / properties");
            }

        public:
            /// @brief Initialize a given buffer
            /// @param graphics_device The graphics (or logical) device instance
            /// @param buffer_size The size to allocate
            /// @param buffer The buffer to allocate
            /// @param buffer_usage Usage flag(s) for the buffer
            /// @param buffer_sharing_mode Sharing mode for the buffer
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initBuffer(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
                const VkDevice& graphics_device,
                const size_t buffer_size,
                VkBuffer& buffer,
                const VkBufferUsageFlags buffer_usage,
                const VkSharingMode buffer_sharing_mode = VK_SHARING_MODE_EXCLUSIVE) noexcept
            {
                VkBufferCreateInfo buffer_create_info{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = buffer_size,
                    .usage = buffer_usage,
                    .sharingMode = buffer_sharing_mode,
                };

                VmaAllocationCreateInfo alloc_info = {
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    .usage = VMA_MEMORY_USAGE_AUTO,
                };

                if (vmaCreateBuffer(resources_allocator, &buffer_create_info, &alloc_info, &buffer, allocation, nullptr) != VK_SUCCESS)
                {
                    LogE("vkCreateBuffer: cannot initiate the buffer with size of %d bytes", buffer_size);
                    return ftstd::VResult::Error((char*)"vkCreateBuffer: cannot initiate the buffer");
                }
                return ftstd::VResult::Ok();
            }
            /// @brief Copy the data from the source buffer to the destination buffer
            /// @param graphics_device The graphics (or logical) device
            /// @param src The source buffer to copy from
            /// @param dst The destination buffer to copy to
            /// @param transfert_command_pool The Transfert command pool
            /// @param size The size of the buffer to copy
            /// @return A VResult type to know if the operation performed well or not
            static ftstd::VResult copyBuffer(
                const VkDevice& graphics_device,
                VkBuffer& src,
                VkBuffer& dst,
                const VkCommandPool* transfert_command_pool,
                const VkQueue& transfert_queue,
                const VkDeviceSize size)
            {
                VkCommandBufferAllocateInfo alloc_info{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = *(transfert_command_pool),
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1};
                VkCommandBuffer command_buffer;
                const auto operation_result = vkAllocateCommandBuffers(graphics_device, &alloc_info, &command_buffer);
                if (operation_result != VK_SUCCESS)
                    return ftstd::VResult::Error((char*)"vkAllocateCommandBuffers failed: cannot allocate memory for the dst buffer");

                // Build the packet
                {
                    VkCommandBufferBeginInfo command_buffer_begin_info{
                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // Wait before submit
                    };
                    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

                    // Specify to copy from 0 to (size - 1)
                    VkBufferCopy copy_region{
                        .srcOffset = 0,
                        .dstOffset = 0,
                        .size = size,
                    };
                    vkCmdCopyBuffer(command_buffer, src, dst, 1, &copy_region);

                    vkEndCommandBuffer(command_buffer);
                }

                // Execute the command buffer to complete the transfert
                VkSubmitInfo submit_info{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &command_buffer,
                };
                vkQueueSubmit(transfert_queue, 1, &submit_info, VK_NULL_HANDLE);
                // Wait operation
                // TODO: use fences next time for opti & multi-sync
                vkQueueWaitIdle(transfert_queue);
                // Clean the command buffer used for the transfert operation
                vkFreeCommandBuffers(graphics_device, *transfert_command_pool, 1, &command_buffer);
                return ftstd::VResult::Ok();
            }
        };
    } // namespace graphics
} // namespace frametech

#endif // memory_h
