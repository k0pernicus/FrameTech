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

frametech::engine::graphics::Texture::Texture() : m_tag("Unknown") {}

frametech::engine::graphics::Texture::~Texture()
{
    const auto resource_allocator = frametech::Engine::getInstance()->m_allocator;
    const auto graphics_device = frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice();
    if (VK_NULL_HANDLE != m_image_view)
    {
        Log("< Destroying the image view object for tag %s...", m_tag.c_str());
        vkDestroyImageView(graphics_device, m_image_view, nullptr);
        m_image_view = VK_NULL_HANDLE;
    }
    if (VK_NULL_HANDLE != m_image)
    {
        Log("< Destroying the image object with tag %s...", m_tag.c_str());
        vmaDestroyImage(resource_allocator, m_image, m_staging_image_allocation);
        m_image = VK_NULL_HANDLE;
        m_staging_image_allocation = VK_NULL_HANDLE;
    }
    if (VK_NULL_HANDLE != m_sampler)
    {
        Log("< Destroying the image sampler with tag %s...", m_tag.c_str());
        vkDestroySampler(graphics_device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

ftstd::VResult frametech::engine::graphics::Texture::createImage(
    const frametech::engine::graphics::Texture::Type texture_type,
    const VkFormat texture_format,
    VmaAllocator resource_allocator) noexcept
{
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

#if defined(DEBUG)
    assert(is_1D_texture ? create_info.extent.depth == 0 : create_info.extent.depth == 1);
#endif

    if (frametech::graphics::Memory::initImage(
            resource_allocator,
            &m_staging_image_allocation,
            m_image,
            create_info)
            .IsError())
    {
        return ftstd::VResult::Error((char*)"Failed to initialize memory for the image");
    }
    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::engine::graphics::Texture::createImageView(
    const frametech::engine::graphics::Texture::Type texture_type,
    const VkFormat texture_format) noexcept
{
    const bool is_1D_texture = texture_type == frametech::engine::graphics::Texture::Type::T1D;

    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = is_1D_texture ? VK_IMAGE_VIEW_TYPE_1D : VK_IMAGE_VIEW_TYPE_2D,
        .format = texture_format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};

    const VkDevice graphics_device = frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice();

    if (const auto result = vkCreateImageView(graphics_device, &view_info, nullptr, &m_image_view); VK_SUCCESS != result)
    {
        return ftstd::VResult::Error((char*)"Failed to initialize the image view");
    }
    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::engine::graphics::Texture::createSampler() noexcept
{
    VkSamplerCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 2.0,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f, 
    };

    const VkDevice graphics_device = frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice();

    if (const auto result = vkCreateSampler(graphics_device, &create_info, nullptr, &m_sampler); VK_SUCCESS != result)
    {
        return ftstd::VResult::Error((char*)"Failed to initialize the sampler");
    }

    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::engine::graphics::Texture::setup(
    char* content,
    const int content_size,
    const bool supports_alpha,
    const frametech::engine::graphics::Texture::Type texture_type,
    const VkFormat texture_format,
    const std::string& tag) noexcept
{
    const int req_comp = supports_alpha ? STBI_rgb_alpha : STBI_rgb;
    m_supports_alpha = supports_alpha;
    m_tag = tag;

    unsigned char* image_data = NULL;

    // Load the data
    if (image_data = stbi_load_from_memory((unsigned char*)content, content_size, &m_width, &m_height, &m_channels, req_comp); nullptr == image_data)
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
        stbi_image_free(image_data);
        return result;
    }

    // Now, fill the staging buffer with the actual data
    void* data;
    vmaMapMemory(resource_allocator, staging_buffer_allocation, &data);
    memcpy(data, image_data, static_cast<size_t>(texture_size));
    vmaUnmapMemory(resource_allocator, staging_buffer_allocation);

    // No need the raw data, as we mapped the data in VMA
    stbi_image_free(image_data);

    if (const auto result = createImage(texture_type, texture_format, resource_allocator); result.IsError())
    {
        LogE("Failed to initialize memory for the image %s", tag.c_str());
        vmaDestroyBuffer(resource_allocator, staging_buffer, staging_buffer_allocation);
        return result;
    }

    if (const auto result = createImageView(texture_type, texture_format); result.IsError())
    {
        LogE("Failed to initialize memory for the image view %s", tag.c_str());
        vmaDestroyBuffer(resource_allocator, staging_buffer, staging_buffer_allocation);
        return result;
    }

    if (const auto result = createSampler(); result.IsError())
    {
        vmaDestroyBuffer(resource_allocator, staging_buffer, staging_buffer_allocation);
        return result;
    }

    // Transition - TOO COMPLEX, REDUCE COMPLEXITY OF TRANSITIONING HERE
    VkQueue transfert_queue = frametech::Engine::getInstance()->m_graphics_device.getTransfertQueue();
    auto transfert_command_buffer = frametech::Engine::getInstance()->m_render->getTransfertCommand();
    VkCommandPool* transfert_command_pool = transfert_command_buffer->getPool();

    const uint32_t transfert_queue_family_index = transfert_command_buffer->m_queue_family_index_created_with;

    {
        frametech::graphics::Command command_buffer(*transfert_command_pool);
        command_buffer.createBuffer();
        command_buffer.begin();
        command_buffer.transition(
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // New
            VK_IMAGE_LAYOUT_UNDEFINED,            // Old
            transfert_queue_family_index,
            transfert_queue_family_index);
        command_buffer.end(transfert_queue, 1);
    }

    frametech::graphics::Memory::copyBufferToImage(
        staging_buffer,
        0,
        m_image,
        transfert_command_pool,
        transfert_queue,
        m_height,
        m_width);

    {
        frametech::graphics::Command command_buffer(*transfert_command_pool);
        command_buffer.createBuffer();
        command_buffer.begin();
        command_buffer.transition(
            m_image,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // New
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // Old
            transfert_queue_family_index,
            transfert_queue_family_index);
        command_buffer.end(transfert_queue, 1);
    }

    vmaDestroyBuffer(resource_allocator, staging_buffer, staging_buffer_allocation);

    return ftstd::VResult::Ok();
}

inline int frametech::engine::graphics::Texture::getTextureSize() const noexcept
{
    const int n_a_channels = m_supports_alpha ? 4 : 3;
    return m_height * m_width * n_a_channels;
}
