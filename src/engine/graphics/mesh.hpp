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

        enum Mesh2D
        {
            BASIC_TRIANGLE = 0,
            BASIC_QUAD = 1,
            NONE = 2,
        };

        struct Mesh
        {
            char m_name[20];
            std::vector<ftstd::shaders::Vertex> m_vertices;
            std::vector<uint32_t> m_indices;
            Mesh2D m_type;
        };

        class MeshUtils
        {
        public:
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
                            .m_indices = indices,
                            .m_vertices = vertices,
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
                            .m_indices = indices,
                            .m_vertices = vertices,
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
                            .m_indices = indices,
                            .m_vertices = vertices,
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
