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
                /// @brief Load the texture data and get the metadata from it
                /// @return A VResult type
                ftstd::VResult setup(
                    const bool supports_alpha,
                    const frametech::engine::graphics::Texture::Type texture_type,
                    const VkFormat texture_format = VK_FORMAT_R8G8B8A8_SRGB) noexcept;
                inline int getTextureSize() const noexcept;

            private:
                int m_height = 0;
                int m_width = 0;
                int m_channels = 0;
                /// @brief Alpha channel by default
                bool m_supports_alpha = true;
                VkImage m_image = VK_NULL_HANDLE;
                VmaAllocation m_staging_image_allocation = VK_NULL_HANDLE;
                /// @brief Takes the filename as a tag
                std::string m_tag;
                Type m_type;
            };
        } // namespace graphics
    }     // namespace engine
} // namespace frametech

#endif // _texture_hpp
