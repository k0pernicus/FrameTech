//
//  memory.hpp
//  FrameTech
//
//  Created by Antonin on 14/12/2022.
//

#pragma once
#ifndef memory_h
#define memory_h

#include "../../ftstd/result.hpp"
#include "command.hpp"
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
            static ftstd::Result<u32> findMemoryType(
                const VkPhysicalDevice& physical_device,
                const u32 type_filter,
                const VkMemoryPropertyFlags memory_property_flags) noexcept
            {
                VkPhysicalDeviceMemoryProperties memory_properties{};
                vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
                for (u32 index = 0; index < memory_properties.memoryTypeCount; ++index)
                {
                    if (type_filter & (1 << index) && (memory_properties.memoryTypes[index].propertyFlags & memory_property_flags) == memory_property_flags)
                    {
                        return ftstd::Result<u32>::Ok(index);
                    }
                }
                return ftstd::Result<u32>::Error((char*)"findMemoryType: did not found any memory type with favorite filter / properties");
            }

        public:
            /// @brief Initialize a given buffer
            /// @param buffer_size The size to allocate
            /// @param buffer The buffer to allocate
            /// @param buffer_usage Usage flag(s) for the buffer
            /// @param buffer_sharing_mode Sharing mode for the buffer
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initBuffer(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
                const int buffer_size,
                VkBuffer& buffer,
                const VkBufferUsageFlags buffer_usage,
                const VkSharingMode buffer_sharing_mode = VK_SHARING_MODE_EXCLUSIVE) noexcept
            {
                VkBufferCreateInfo buffer_create_info{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = static_cast<VkDeviceSize>(buffer_size),
                    .usage = buffer_usage,
                    .sharingMode = buffer_sharing_mode,
                };

                VmaAllocationCreateInfo alloc_info = {
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
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
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initImage(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
                VkImage& image,
                VkImageCreateInfo& image_create_info) noexcept
            {
                VmaAllocationCreateInfo alloc_info = {
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    // Windows build : check why VMA_MEMORY_USAGE_AUTO does not work on Windows...
                    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
                };

                if (vmaCreateImage(resources_allocator, &image_create_info, &alloc_info, &image, allocation, nullptr) != VK_SUCCESS)
                {
                    LogE("vmaCreateImage: cannot initiate the image");
                    return ftstd::VResult::Error((char*)"vmaCreateImage: cannot initiate the image");
                }
                return ftstd::VResult::Ok();
            }
            
            /// @brief Copy the data from the source image to the destination VkImage, only for depth texture
            /// @param resources_allocator Allocator for the resources
            /// @param allocation Allocation structure (out)
            /// @param image A reference to a VkImage
            /// @param image_create_info Create info structure about the image
            /// @return A VResult type to know if the initialization succeeded or not
            static ftstd::VResult initDepthImage(
                VmaAllocator& resources_allocator,
                VmaAllocation* allocation,
                VkImage& image,
                VkImageCreateInfo& image_create_info) noexcept
            {
                VmaAllocationCreateInfo alloc_info = {
                    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
                    // Windows build : check why VMA_MEMORY_USAGE_AUTO does not work on Windows...
                    .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
                };

                if (vmaCreateImage(resources_allocator, &image_create_info, &alloc_info, &image, allocation, nullptr) != VK_SUCCESS)
                {
                    LogE("vmaCreateImage: cannot initiate the image");
                    return ftstd::VResult::Error((char*)"vmaCreateImage: cannot initiate the image");
                }
                return ftstd::VResult::Ok();
            }

            /// @brief Copy the data from the source buffer to the destination buffer
            /// @param src The source buffer to copy from
            /// @param src_offset Offset of the source buffer to copy from (default should be 0)
            /// @param dst The destination buffer to copy to
            /// @param dst_offset Offset of the destination buffer to copy to (default should be 0)
            /// @param transfert_command_pool The Transfert command pool
            /// @param size The size of the buffer to copy
            /// @return A VResult type to know if the operation performed well or not
            static ftstd::VResult copyBufferToBuffer(
                VkBuffer& src,
                VkDeviceSize src_offset,
                VkBuffer& dst,
                VkDeviceSize dst_offset,
                VkCommandPool* transfert_command_pool,
                const VkQueue& transfert_queue,
                const VkDeviceSize size)
            {

                frametech::graphics::Command command_buffer(*transfert_command_pool);
                command_buffer.createBuffer();
                command_buffer.begin();

                u32 submit_count = 0;

                // Build the packet
                {
                    VkBufferCopy copy_region{
                        .srcOffset = src_offset,
                        .dstOffset = dst_offset,
                        .size = size,
                    };
                    vkCmdCopyBuffer(*command_buffer.getBuffer(), src, dst, 1, &copy_region);
                    ++submit_count;
                }

                command_buffer.end(transfert_queue, submit_count);
                return ftstd::VResult::Ok();
            }

            /// @brief Copy the data from the source buffer to the VkImage destination
            /// @param src_buffer The source buffer to copy from
            /// @param src_offset Offset of the source buffer to copy from (default should be 0)
            /// @param dst_image The VkImage destination
            /// @param transfert_command_pool The Transfert command pool
            /// @param image_height The height of the image to copy in
            /// @param image_width The width of the image to copy in
            /// @return A VResult type to know if the operation performed well or not
            static ftstd::VResult copyBufferToImage(
                VkBuffer& src_buffer,
                VkDeviceSize src_offset,
                VkImage& dst_image,
                VkCommandPool* transfert_command_pool,
                const VkQueue& transfert_queue,
                const u32 image_height,
                const u32 image_width)
            {

                frametech::graphics::Command command_buffer(*transfert_command_pool);
                command_buffer.createBuffer();
                command_buffer.begin();

                u32 submit_count = 0;

                // Build the packet
                {
                    VkBufferImageCopy copy_region{
                        .bufferOffset = src_offset,
                        .bufferRowLength = 0,
                        .bufferImageHeight = 0,
                        .imageSubresource = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                        .imageOffset = {0, 0, 0},
                        .imageExtent = {.width = image_width, .height = image_height, .depth = 1}};
                    vkCmdCopyBufferToImage(
                        *command_buffer.getBuffer(),
                        src_buffer,
                        dst_image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &copy_region);
                    ++submit_count;
                }

                command_buffer.end(transfert_queue, submit_count);
                return ftstd::VResult::Ok();
            }
        };
    } // namespace graphics
} // namespace frametech

#endif // memory_h
