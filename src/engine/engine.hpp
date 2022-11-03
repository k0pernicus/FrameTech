//
//  engine.hpp
//  FrameTech
//
//  Created by Antonin on 24/09/2022.
//

#pragma once
#ifndef engine_hpp
#define engine_hpp

#include "../ftstd/result.h"
#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/render.hpp"
#include "graphics/swapchain.hpp"
#include "vulkan/vulkan.h"
#include <cstdlib>

namespace FrameTech
{

    class Engine
    {
    public:
        /// @brief Internal state of the Engine object,
        /// to know if the unique object needs
        /// to be initialized or not
        enum State
        {
            /// @brief The starting state
            UNINITIALIZED,
            /// @brief The graphics pipeline and state
            /// machine have been initialized - it is
            /// now ready to use
            INITIALIZED,
            /// @brief An error happened during the initialization,
            /// the application should not run
            ERROR,
        };

    private:
        Engine();
        /// @brief Engine should not be cloneable
        Engine(Engine& other) = delete;
        /// @brief Engine should not be assignable
        void operator=(const Engine& other) = delete;
        /// @brief The single internal instance of the Engine object
        static Engine* m_instance;
        /// @brief Creates the Vulkan instance of the Engine
        VResult createGraphicsInstance();
        /// @brief Choose and picks a physical device
        VResult pickPhysicalDevice();
        /// @brief Creates the render device
        VResult createRenderDevice();
        /// @brief Creates the swapchain
        VResult createSwapChain();
        /// @brief Stores the internal state of the unique
        /// Engine object
        FrameTech::Engine::State m_state;

    public:
        /// @brief Get the singleton Engine object
        /// @return A static pointer to the Engine unique object
        static Engine* getInstance();
        /// @brief Initializes the Engine object
        /// only if the Engine object is in
        /// UNINITIALIZED state
        void initialize();
        /// @brief Returns the internal state of the unique
        /// Engine object
        FrameTech::Engine::State getState();
        ~Engine();

        /// @brief The engine instance
        VkInstance m_graphics_instance = NULL;
        /// @brief The physical device
        FrameTech::Graphics::Device m_graphics_device = FrameTech::Graphics::Device();
        /// @brief The renderer of the engine
        std::unique_ptr<FrameTech::Graphics::Render> m_render;
        /// @brief The swapchain of the engine
        std::unique_ptr<FrameTech::Graphics::SwapChain> m_swapchain;
        ;
    };

} // namespace FrameTech

#endif // engine_hpp
