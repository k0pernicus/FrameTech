//
//  render.hpp
//  FrameTech
//
//  Created by Antonin on 28/09/2022.
//

#pragma once
#ifndef render_h
#define render_h

#include "../../ftstd/result.h"
#include "pipeline.hpp"
#include "vulkan/vulkan.h"
#include <vector>

namespace FrameTech
{
    namespace Graphics
    {
        /// @brief Contains a basic Renderer
        class Render
        {
        public:
            /// @brief Returns the static instance (singleton) of the object
            /// @return A static Render pointer
            static Render* getInstance();
            /// @brief Destructor
            ~Render();
            /// @brief Presents rendered images to a window.
            /// @return A Result type to know if the surface abstraction
            /// has been created.
            VResult createSurface();
            /// @brief Returns the KHR surface as a pointer
            VkSurfaceKHR* getSurface();
            /// @brief Creates the image views for the Render, from the
            /// images from the SwapChain object.
            /// @return A Result type to know if the function succeeded
            /// or not.
            VResult createImageViews();
            /// @brief Creates the graphics pipeline:
            /// 1. Read the SPIR-V shaders,
            /// 2. Create the shader modules,
            /// 3. Create the shader stages.
            /// @return A Result type to know if the function succeeded
            /// or not.
            VResult createGraphicsPipeline();

        private:
            /// @brief Constructor
            Render();
            /// @brief Render should not be cloneable
            Render(Render& other) = delete;
            /// @brief Render should not be assignable
            void operator=(const Render& other) = delete;
            /// @brief The default surface to present rendered images
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            /// @brief The internal instance (singleton) of
            /// the Render object
            static Render* m_instance;
            /// @brief Literal views to different images - describe how
            /// to access images and which part of the images to access
            std::vector<VkImageView> m_image_views;
            /// @brief The graphics pipeline, associated to a Renderer
            /// TODO: check if the graphics pipeline belongs exclusively
            /// to the Render object or if it belongs to the engine directly
            std::unique_ptr<FrameTech::Graphics::Pipeline> m_graphics_pipeline = nullptr;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif // render_h
