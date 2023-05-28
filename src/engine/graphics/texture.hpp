//
//  texture.hpp
//  FrameTech
//
//  Created by Antonin on 27/05/2023.
//

#pragma once
#ifndef _texture_hpp
#define _texture_hpp

#include <cstdint>
#include <stb/stb_image.h>
#include <string>
#include "result.hpp"
#include <vulkan/vulkan.h>

#include "../../engine/engine.hpp"

namespace frametech
{
    /// @brief Handles a Texture object
    class Texture {
    public:
        /// @brief No support for 3D textures yet
        enum struct Type { 
            T1D, // 1D texture
            T2D, // 2D texture
        };
        Texture(const std::string& filename);
        ~Texture() {
            const auto resource_allocator = frametech::Engine::getInstance()->m_allocator;
            if (VK_NULL_HANDLE != m_image)
            {
                Log("< Destroying the image object...");
                vmaDestroyImage(resource_allocator, m_image, m_staging_image_allocation);
                m_image = VK_NULL_HANDLE;
                m_staging_image_allocation = VK_NULL_HANDLE;
            }
        }
        /// @brief Load the texture data and get the metadata from it
        /// @return A VResult type
        ftstd::VResult setup(
            const bool supports_alpha, 
            const frametech::Texture::Type texture_type,
            const VkFormat texture_format = VK_FORMAT_R8G8B8A8_SRGB
        ) noexcept;
        inline int getTextureSize() const noexcept;

    private:
        int m_height = 0;
        int m_width = 0;
        int m_channels = 0;
        bool m_supports_alpha = true; // Alpha channel by default
        VkImage m_image = VK_NULL_HANDLE;
        VmaAllocation m_staging_image_allocation = VK_NULL_HANDLE;
        std::string m_tag;
        Type m_type;
    };
} // namespace ftstd

#endif // _texture_hpp
