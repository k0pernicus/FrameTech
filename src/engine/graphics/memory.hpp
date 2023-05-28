//
//  memory.hpp
//  FrameTech
//
//  Created by Antonin on 14/12/2022.
//

#pragma once
#ifndef memory_h
#define memory_h

#include "command.hpp"
#include "../../ftstd/result.hpp"
#include <assert.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace graphics
    {
        class Memory
        {
        public:
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

            /// @brief Initialize a given buffer
            /// @param buffer_size The size to allocate
            /// @param buffer The buffer to allocate
            /// @param buffer_usage Usage flag(s) for the buffer
            /// @param buffer_sharing_mode Sharing mode for the buffer
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initBuffer(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
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
                    LogE("vmaCreateBuffer: cannot initiate the buffer with size of %d bytes", buffer_size);
                    return ftstd::VResult::Error((char*)"vmaCreateBuffer: cannot initiate the buffer");
                }
                return ftstd::VResult::Ok();
            }

            /// @brief Copy the data from the source image to the destination VkImage
            /// @param resources_allocator Allocator for the resources
            /// @param allocation Allocation structure (out)
            /// @param image A reference to a VkImage
            /// @param image_create_info Create info structure about the image
            /// @param buffer_sharing_mode Sharing mode for the image
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initImage(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
                VkImage& image,
                VkImageCreateInfo& image_create_info) noexcept
            {
                VmaAllocationCreateInfo alloc_info = {
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    .usage = VMA_MEMORY_USAGE_AUTO,
                };

                if (vmaCreateImage(resources_allocator, &image_create_info, &alloc_info, &image, allocation, nullptr) != VK_SUCCESS)
                {
                    LogE("vmaCreateImage: cannot initiate the image");
                    return ftstd::VResult::Error((char*)"vmaCreateImage: cannot initiate the image");
                }
                return ftstd::VResult::Ok();
            }

            /// @brief Copy the data from the source buffer to the destination buffer
            /// @param graphics_device The graphics (or logical) device
            /// @param src The source buffer to copy from
            /// @param src_offset Offset of the source buffer to copy from (default should be 0)
            /// @param dst The destination buffer to copy to
            /// @param dst_offset Offset of the destination buffer to copy to (default should be 0)
            /// @param transfert_command_pool The Transfert command pool
            /// @param size The size of the buffer to copy
            /// @return A VResult type to know if the operation performed well or not
            static ftstd::VResult copyBuffer(
                const VkDevice& graphics_device,
                VkBuffer& src,
                VkDeviceSize src_offset,
                VkBuffer& dst,
                VkDeviceSize dst_offset,
                VkCommandPool* transfert_command_pool,
                const VkQueue& transfert_queue,
                const VkDeviceSize size)
            {
                frametech::graphics::Command command_buffer(transfert_command_pool);
                command_buffer.begin();

                uint32_t submit_count = 1;

                // Build the packet
                {
                    // Specify to copy from 0 to (size - 1)
                    VkBufferCopy copy_region{
                        .srcOffset = src_offset,
                        .dstOffset = dst_offset,
                        .size = size,
                    };
                    vkCmdCopyBuffer(*command_buffer.getBuffer(), src, dst, 1, &copy_region);
                }

                command_buffer.end(transfert_queue, submit_count);
                return ftstd::VResult::Ok();
            }
        };
    } // namespace graphics
} // namespace frametech

#endif // memory_h
