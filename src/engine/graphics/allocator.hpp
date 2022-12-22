//
//  allocator.hpp
//  FrameTech
//
//  Created by Antonin on 14/12/2022.
//

#pragma once
#ifndef allocator_h
#define allocator_h

#include "../../ftstd/result.h"
#include <assert.h>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        class Allocator
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
                if (vkCreateBuffer(graphics_device, &buffer_create_info, nullptr, &buffer) != VK_SUCCESS)
                {
                    LogE("vkCreateBuffer: cannot initiate the buffer with size of %d bytes", buffer_size);
                    return ftstd::VResult::Error((char*)"vkCreateBuffer: cannot initiate the buffer");
                }
                return ftstd::VResult::Ok();
            }
            /// @brief Allocate memory to buffer
            /// @param physical_device The physical device instance
            /// @param graphics_device The graphics (or logical) device instance
            /// @param buffer_memory The buffer memory device instance
            /// @param buffer The buffer to bind
            /// @param memory_offset Memory offset (default to 0)
            /// @return A VResult type to know if the allocation succeeded or not
            static ftstd::VResult allocateMemoryToBuffer(
                const VkPhysicalDevice& physical_device,
                const VkDevice& graphics_device,
                VkDeviceMemory& buffer_memory,
                VkBuffer& buffer,
                const VkMemoryPropertyFlags memory_properties,
                const VkDeviceSize memory_offset = 0) noexcept
            {
                VkMemoryRequirements memory_requirements{};
                vkGetBufferMemoryRequirements(graphics_device, buffer, &memory_requirements);
#ifdef DEBUG
                assert(memory_requirements.size > 0 && memory_requirements.alignment > 0);
#endif
                if (memory_requirements.size <= 0 || memory_requirements.alignment <= 0)
                {
                    return ftstd::VResult::Error((char*)"vkGetBufferMemoryRequirements: memory requirements structure is invalid: size or alignment <= 0");
                }

                // Check for the memory type index
                const ftstd::Result<uint32_t> memory_type_index = findMemoryType(
                    physical_device,
                    memory_requirements.memoryTypeBits,
                    memory_properties);

                if (memory_type_index.IsError())
                    return ftstd::VResult::Error((char*)"allocateMemoryToBuffer: cannot find the memory type index found via the memory requirements structure");

                // Alloc infos
                VkMemoryAllocateInfo alloc_info{};
                alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                alloc_info.allocationSize = memory_requirements.size;
                alloc_info.memoryTypeIndex = memory_type_index.GetValue();

                // TODO: Update vkAllocateMemory with a proper custom allocator + offset in the future
                if (const auto result = vkAllocateMemory(graphics_device, &alloc_info, nullptr, &buffer_memory); result != VK_SUCCESS)
                {
                    return ftstd::VResult::Error((char*)"vkAllocateMemory: cannot allocate memory for the buffer passed as parameter");
                }

                vkBindBufferMemory(graphics_device, buffer, buffer_memory, memory_offset);

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

#endif // allocator_h
