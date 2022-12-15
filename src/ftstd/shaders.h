//
//  shaders.h
//  FrameTech
//
//  Created by Antonin on 13/12/2022.
//

#pragma once
#ifndef shaders_h
#define shaders_h

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace ftstd
{
    namespace shaders
    {
        struct Vertex
        {
            /// @brief Vertex position
            glm::vec2 position;
            /// @brief Vertex color
            glm::vec3 color;
        };

        class VertexUtils
        {
        public:
            [[maybe_unused]] static void toString(char* str, const Vertex& vertex) noexcept
            {
                snprintf(str, 80, "Position: (%.2f,%.2f)\nColor: (%d,%d,%d)", vertex.position.x, vertex.position.y, static_cast<int>(255.0 * vertex.color.r), static_cast<int>(255.0 * vertex.color.g), static_cast<int>(255.0 * vertex.color.b));
            }
            /// @brief Returns the binding description of the Vertex structure
            /// @param index_binding The binding index to set in the description data structure
            /// @return a VkVertexInputBindingDescription object
            [[maybe_unused]] static VkVertexInputBindingDescription getVertexBindingDescription(uint32_t index_binding = 0) noexcept
            {
                VkVertexInputBindingDescription binding_description{
                    .binding = index_binding,                 // index of the binding in the overall array
                    .stride = sizeof(Vertex),                 // specifies the number of bytes from one-entry to the next
                    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // move to the next data entry after each vertex
                };
                return binding_description;
            }
            /// @brief Returns an array of attribute descriptions of the Vertex structure
            /// @return an array of length 2: position and color of the shader (in this particular order)
            [[maybe_unused]] static std::array<VkVertexInputAttributeDescription, 2> getVertexAttributeDescriptions() noexcept
            {
                /* Attribute description type of data
                 * ----------------------------------
                 * float: VK_FORMAT_R32_SFLOAT
                 * vec2: VK_FORMAT_R32G32_SFLOAT
                 * vec3: VK_FORMAT_R32G32B32_SFLOAT
                 * vec4: VK_FORMAT_R32G32B32A32_SFLOAT
                 * */
                std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
                attribute_descriptions[0] = VkVertexInputAttributeDescription{
                    .binding = 0,
                    .location = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT, // Position -> 2 floats
                    .offset = offsetof(Vertex, position),
                };
                attribute_descriptions[1] = VkVertexInputAttributeDescription{
                    .binding = 0,
                    .location = 1,
                    .format = VK_FORMAT_R32G32B32_SFLOAT, // Color -> 3 floats
                    .offset = offsetof(Vertex, color),
                };
                return attribute_descriptions;
            }
        };
    } // namespace shaders
} // namespace ftstd

#endif // shaders_h
