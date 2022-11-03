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
#include "commandbuffer.hpp"
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
            /// @brief Returns the framebuffers
            std::vector<VkFramebuffer> getFramebuffers();
            /// @brief Creates the image views for the Render, from the
            /// images from the SwapChain object.
            /// @return A VResult type to know if the function succeeded
            /// or not.
            VResult createImageViews();
            /// @brief Creates the framebuffers for the objects to render
            /// @return A VResult type to know if the function succeeded
            /// or not.
            VResult createFramebuffers();
            /// @brief Creates the graphics pipeline
            /// @return A VResult type to know if the function succeeded
            /// or not.
            VResult createGraphicsPipeline();
            /// @brief Returns the frame index, or swap chain index
            /// @return The current frame index
            uint64_t getFrameIndex();
            /// @brief Updates the current frame, or swap chain, index
            /// Should not be called more than once per frame present
            void updateFrameIndex(uint64_t current_frame);
            /// @brief Returns the associated CommandBuffer object if it exists
            std::shared_ptr<FrameTech::Graphics::CommandBuffer> getCommandBuffer() const;
            /// @brief Returns the associated Graphics pipeline object if it exists
            std::shared_ptr<FrameTech::Graphics::Pipeline> getGraphicsPipeline() const;

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
            /// @brief Reference all of the VkImageView objects
            std::vector<VkFramebuffer> m_framebuffers;
            /// @brief The graphics pipeline, associated to a Renderer
            std::shared_ptr<FrameTech::Graphics::Pipeline> m_graphics_pipeline = nullptr;
            /// @brief Renders command pool
            std::shared_ptr<FrameTech::Graphics::CommandBuffer> m_command_buffer = nullptr;
            /// @brief Creates the shader module:
            /// 1. Read the SPIR-V shaders,
            /// 2. Create the shader modules,
            /// 3. Create the shader stages.
            /// @return A Result type to know if the function succeeded
            /// or not.
            VResult createShaderModule();
            /// @brief The current frame index, or swap chain index
            uint64_t m_frame_index = 0;
        };
    } // namespace Graphics
} // namespace FrameTech

#endif // render_h
