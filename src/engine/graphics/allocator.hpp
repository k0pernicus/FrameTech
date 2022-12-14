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
            static ftstd::Result<uint32_t> findMemoryType(const VkPhysicalDevice& physical_device, const uint32_t type_filter, const VkMemoryPropertyFlags memory_property_flags) noexcept
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
            static ftstd::VResult initBuffer(const VkDevice& graphics_device, const size_t buffer_size, VkBuffer& buffer, const VkBufferUsageFlags buffer_usage, const VkSharingMode buffer_sharing_mode = VK_SHARING_MODE_EXCLUSIVE) noexcept
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
                    // The most optimal setting for a non-dedicated device is
                    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                    // TODO: check if the application is running on a dedicated GPU or not and,
                    // according to this, switch from one to the other
                    // M1 / M2 macs are fine for this configuration, which should not work for any other
                    // non-ARM macs (or linux / windows machines)
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                if (memory_type_index.IsError())
                    return ftstd::VResult::Error((char*)"allocateMemoryToBuffer: cannot find the memory type index found via the memory requirements structure");

                // Alloc infos
                VkMemoryAllocateInfo alloc_info{};
                alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                alloc_info.allocationSize = memory_requirements.size;
                alloc_info.memoryTypeIndex = memory_type_index.GetValue();

                if (const auto result = vkAllocateMemory(graphics_device, &alloc_info, nullptr, &buffer_memory); result != VK_SUCCESS)
                {
                    return ftstd::VResult::Error((char*)"vkAllocateMemory: cannot allocate memory for the buffer passed as parameter");
                }

                vkBindBufferMemory(graphics_device, buffer, buffer_memory, memory_offset);

                return ftstd::VResult::Ok();
            }
        };
    } // namespace graphics
} // namespace frametech

#endif // allocator_h
