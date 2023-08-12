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
#include <objloader/objloader.h>
#include <vulkan/vulkan.h>

namespace frametech
{
    namespace engine
    {
        namespace graphics
        {
            namespace shaders
            {
                struct Vertex
                {
                    /// @brief Vertex position - 3D mode (XYZ)
                    glm::vec3 m_position;
                    /// @brief Vertex color - 3D mode (RGB)
                    glm::vec3 m_color;
                    /// @brief Texture UV - 2D mode (UV)
                    glm::vec2 m_texture_coordinates;

                    void setColor(const glm::vec3& new_color)
                    {
                        m_color = new_color;
                    }

                    void setPosition(const glm::vec3& new_position)
                    {
                        m_position = new_position;
                    }

                    void setTextureCoordinates(const glm::vec2& new_coordinates)
                    {
                        m_texture_coordinates = new_coordinates;
                    }
                    
                    static Vertex fromObjLoaderVertex(const objl::Vertex& v) noexcept {
                        Vertex vertex {
                            .m_position = glm::vec3(v.Position.X, v.Position.Y, v.Position.Z),
                            .m_color = glm::vec3(v.Normal.X, v.Normal.Y, v.Normal.Z),
                            .m_texture_coordinates = glm::vec2(v.TextureCoordinate.X, v.TextureCoordinate.Y),
                        };
                        return vertex;
                    }
                };

                class VertexUtils
                {
                public:
                    [[maybe_unused]] static void toString(char* str, const Vertex& vertex) noexcept
                    {
                        snprintf(str, 80, "Position: (%.2f,%.2f,%.2f)\nColor: (%d,%d,%d)", vertex.m_position.x, vertex.m_position.y, vertex.m_position.z, static_cast<int>(255.0 * vertex.m_color.r), static_cast<int>(255.0 * vertex.m_color.g), static_cast<int>(255.0 * vertex.m_color.b));
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
                    [[maybe_unused]] static std::array<VkVertexInputAttributeDescription, 3> getVertexAttributeDescriptions() noexcept
                    {
                        /* Attribute description type of data
                         * ----------------------------------
                         * 0 : 3D vertices (XYZ)
                         * 1 : Colors (RGB)
                         * 2 : 2D UVs (XY)
                         * */
                        std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
                        // Vertices pos
                        attribute_descriptions[0] = VkVertexInputAttributeDescription{
                            .location = 0,
                            .binding = 0,
                            .format = VK_FORMAT_R32G32B32_SFLOAT, // Position -> 2 floats
                            .offset = offsetof(Vertex, m_position),
                        };
                        // Colors
                        attribute_descriptions[1] = VkVertexInputAttributeDescription{
                            .location = 1,
                            .binding = 0,
                            .format = VK_FORMAT_R32G32B32_SFLOAT, // Color -> 3 floats
                            .offset = offsetof(Vertex, m_color),
                        };
                        // Texture attributes (coordinates : UV)
                        attribute_descriptions[2] = VkVertexInputAttributeDescription{
                            .location = 2,
                            .binding = 0,
                            .format = VK_FORMAT_R32G32_SFLOAT,
                            .offset = offsetof(Vertex, m_texture_coordinates),
                        };
                        return attribute_descriptions;
                    }
                };
            } // namespace shaders
        }     // namespace graphics
    }         // namespace engine
} // namespace frametech

#endif // shaders_h
