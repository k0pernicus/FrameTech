//
//  texture.hpp
//  FrameTech
//
//  Created by Antonin on 27/05/2023.
//

#pragma once
#ifndef _texture_hpp
#define _texture_hpp

#include "../ftstd/result.hpp"
#include <cstdint>
#include <stb/stb_image.h>
#include <string>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "../../engine/engine.hpp"

namespace frametech
{
    namespace engine
    {
        namespace graphics
        {
            /// @brief Handles a Texture object
            class Texture
            {
            public:
                /// @brief No support for 3D textures yet
                enum struct Type
                {
                    T1D, // 1D texture
                    T2D, // 2D texture
                };
                Texture();
                ~Texture();
                Texture(Texture const&) = delete;
                Texture& operator=(Texture const&) = delete;
                /// @brief Load the compressed texture data and get the metadata from it
                /// @return A VResult type
                ftstd::VResult setup(
                    char* content,
                    const int content_size,
                    const bool supports_alpha,
                    const frametech::engine::graphics::Texture::Type texture_type,
                    const VkFormat texture_format = VK_FORMAT_R8G8B8A8_SRGB,
                    const std::string& tag = "Unknown") noexcept;
                inline int getTextureSize() const noexcept;
                /// @brief Returns a copy of the registered sampler
                /// @return VkSampler
                VkSampler getSampler() noexcept { return m_sampler; }
                /// @brief Returns a copy of the registered image view
                /// @return VkImageView
                VkImageView getImageView() noexcept { return m_image_view; }

            private:
                /// @brief Height of the texture image
                int m_height = 0;
                /// @brief Width of the texture image
                int m_width = 0;
                /// @brief Number of channels in the texture image
                int m_channels = 0;
                /// @brief Alpha channel by default
                bool m_supports_alpha = true;
                /// @brief Vulkan image object
                VkImage m_image = VK_NULL_HANDLE;
                /// @brief Vulkan image view to access the texture for the GPU
                VkImageView m_image_view = VK_NULL_HANDLE;
                /// @brief The sampler associated to the texture
                VkSampler m_sampler = VK_NULL_HANDLE;
                /// @brief Resource allocation object, required for VMA
                VmaAllocation m_staging_image_allocation = VK_NULL_HANDLE;
                /// @brief Creates the VkImage of the current object
                /// @return As a result
                ftstd::VResult createImage(const frametech::engine::graphics::Texture::Type texture_type,
                                           const VkFormat texture_format,
                                           VmaAllocator resource_allocator) noexcept;
                /// @brief Creates the VkImageView of the current object
                /// @return As a result
                ftstd::VResult createImageView(const frametech::engine::graphics::Texture::Type texture_type,
                                               const VkFormat texture_format) noexcept;
                /// @brief Creates the VkSampler of the current object
                /// @return As a result
                ftstd::VResult createSampler() noexcept;
                /// @brief Takes the filename as a tag
                std::string m_tag;
                Type m_type;
            };
        } // namespace graphics
    }     // namespace engine
} // namespace frametech

#endif // _texture_hpp
