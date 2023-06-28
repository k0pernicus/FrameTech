//
//  texture.cpp
//  FrameTech
//
//  Created by Antonin on 27/05/2023.
//

#include "texture.hpp"
#include "debug_tools.h"
#include "memory.hpp"
#include "vma/vk_mem_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

frametech::engine::graphics::Texture::Texture(const std::string& filename) : m_tag(filename)
{
}

frametech::engine::graphics::Texture::~Texture()
{
    const auto resource_allocator = frametech::Engine::getInstance()->m_allocator;
    if (VK_NULL_HANDLE != m_image)
    {
        Log("< Destroying the image object...");
        vmaDestroyImage(resource_allocator, m_image, m_staging_image_allocation);
        m_image = VK_NULL_HANDLE;
        m_staging_image_allocation = VK_NULL_HANDLE;
    }
}

ftstd::VResult frametech::engine::graphics::Texture::setup(
    const bool supports_alpha,
    const frametech::engine::graphics::Texture::Type texture_type,
    const VkFormat texture_format) noexcept
{
    const int req_comp = supports_alpha ? STBI_rgb_alpha : STBI_rgb;
    m_supports_alpha = supports_alpha;
    void* content = nullptr;
    // Load the data
    if (content = stbi_load(m_tag.c_str(), &m_width, &m_height, &m_channels, req_comp); nullptr == content)
    {
        LogE("Cannot load the texture with name '%s', should not happen", m_tag.c_str());
        return ftstd::VResult::Error((char*)"Error loading texture data");
    }

    m_type = texture_type;
    // Now, create the texture image & memory
    VmaAllocator resource_allocator = frametech::Engine::getInstance()->m_allocator;
    const int texture_size = getTextureSize();
    // Use staging buffer (or temporary buffer) to transfer next from CPU to GPU
    // This buffer can be used as source in a memory transfer operation
    VkBuffer staging_buffer{};
    VmaAllocation staging_buffer_allocation{};
    if (const auto result = frametech::graphics::Memory::initBuffer(
            resource_allocator,
            &staging_buffer_allocation,
            texture_size,
            staging_buffer,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        result.IsError())
    {
        LogE("Cannot initialize the buffer");
        stbi_image_free(content);
        return result;
    }

    // Now, fill the staging buffer with the actual data
    void* data;
    vmaMapMemory(resource_allocator, staging_buffer_allocation, &data);
    memcpy(data, content, static_cast<size_t>(texture_size));
    vmaUnmapMemory(resource_allocator, staging_buffer_allocation);

    // No need the raw data, as we mapped the data in VMA
    stbi_image_free(content);

    const bool is_1D_texture = texture_type == frametech::engine::graphics::Texture::Type::T1D;

    VkImageCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = is_1D_texture ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D,
        .format = texture_format,
        .extent = {
            .width = static_cast<uint32_t>(m_width),
            .height = static_cast<uint32_t>(m_height),
            .depth = is_1D_texture ? static_cast<uint32_t>(0) : static_cast<uint32_t>(1), // FIXME !!! Big errors will happen for 3D textures - handle the depth via the image load function,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,  // Related to multisampling
        .tiling = VK_IMAGE_TILING_OPTIMAL, // TODO: switch maybe to Optimal in the future, or let the dev decides of it
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,   // Used by one queue family, the one that supports graphics
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // first transition will discard the texels
    };

    if (frametech::graphics::Memory::initImage(
            resource_allocator,
            &m_staging_image_allocation,
            m_image,
            create_info)
            .IsError())
    {
        return ftstd::VResult::Error((char*)"Failed to initialize memory for the image");
    }

    // Finally, bind the image
    vmaBindImageMemory(resource_allocator, m_staging_image_allocation, m_image);

    return ftstd::VResult::Ok();
}

inline int frametech::engine::graphics::Texture::getTextureSize() const noexcept
{
    const int n_a_channels = m_supports_alpha ? 4 : 3;
    return m_height * m_width * n_a_channels;
}
