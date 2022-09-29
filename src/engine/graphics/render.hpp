//
//  render.hpp
//  FrameTech
//
//  Created by Antonin on 28/09/2022.
//

#pragma once
#ifndef render_h
#define render_h

#include "../../result.h"
#include "vulkan/vulkan.h"

namespace FrameTech
{
    /// @brief Contains a basic Renderer
    class Render
    {
    public:
        static Render* getInstance();
        /// @brief Destructor
        ~Render();
        /// @brief Presents rendered images to a window.
        /// @return A Result type to know if the surface abstraction
        /// has been created.
        Result<int> createSurface();
        /// @brief Returns the KHR surface as a pointer
        VkSurfaceKHR* getSurface();

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
        static Render* m_render_instance;
    };
} // namespace FrameTech

#endif // render_h
