//
//  mesh.hpp
//  FrameTech
//
//  Created by Antonin on 15/12/2022.
//

#pragma once
#ifndef mesh_h
#define mesh_h

#include "../../ftstd/shaders.h"
#include <vector>

namespace frametech
{
    namespace graphics
    {
        /// @brief Mesh 2D IDs
        enum Mesh2D
        {
            BASIC_TRIANGLE = 0,
            BASIC_QUAD = 1,
            NONE = 2,
        };

        /// @brief Stores all information about
        /// a Mesh object
        struct Mesh
        {
            /// @brief The name / tag of the Mesh object
            char m_name[20];
            /// @brief The vertices of the Mesh object
            std::vector<ftstd::shaders::Vertex> m_vertices;
            /// @brief The indices of the Mesh object
            std::vector<uint32_t> m_indices;
            /// @brief The type of the Mesh object
            Mesh2D m_type;
        };

        /// @brief Utility class to interact with Mesh objects
        class MeshUtils
        {
        public:
            /// @brief Returns a 2D Mesh object, based on the id passed as parameter
            /// @param queried_mesh The 2D Mesh ID to get
            /// @return A Mesh object
            static Mesh getMesh2D(const Mesh2D queried_mesh) noexcept
            {
                switch (queried_mesh)
                {
                    case Mesh2D::BASIC_QUAD:
                    {
                        //  Indices
                        std::vector<uint32_t> indices(6);
                        indices = {0, 1, 2, 2, 3, 0};
                        // Vertices
                        std::vector<ftstd::shaders::Vertex> vertices(4);
                        vertices = {
                            {{-1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
                            {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                            {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
                            {{-1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}};
                        auto mesh = Mesh{
                            .m_vertices = vertices,
                            .m_indices = indices,
                            .m_type = Mesh2D::BASIC_QUAD};
                        snprintf(mesh.m_name, 20, "basic_quad");
                        return mesh;
                    }
                    case Mesh2D::BASIC_TRIANGLE:
                    {
                        //  Indices
                        std::vector<uint32_t> indices(3);
                        indices = {0, 1, 2};
                        // Vertices
                        std::vector<ftstd::shaders::Vertex> vertices(3);
                        vertices = {
                            {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
                            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
                        auto mesh = Mesh{
                            .m_vertices = vertices,
                            .m_indices = indices,
                            .m_type = Mesh2D::BASIC_TRIANGLE};
                        snprintf(mesh.m_name, 20, "basic_triangle");
                        return mesh;
                    }
                    case Mesh2D::NONE:
                    {
                        // Indices
                        std::vector<uint32_t> indices(1);
                        indices = {0};
                        // Vertices
                        std::vector<ftstd::shaders::Vertex> vertices(0);
                        vertices = {{{0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}};
                        auto mesh = Mesh{
                            .m_vertices = vertices,
                            .m_indices = indices,
                            .m_type = Mesh2D::NONE};
                        snprintf(mesh.m_name, 20, "None");
                        return mesh;
                    }
                }
            }
        };
    } // namespace graphics
} // namespace frametech

#endif // mesh_h
