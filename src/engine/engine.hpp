//
//  engine.hpp
//  FrameTech
//
//  Created by Antonin on 24/09/2022.
//

#pragma once
#ifndef engine_hpp
#define engine_hpp

#include "../result.h"
#include "pipeline.hpp"
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
        Result<int> createGraphicsInstance();
        /// @brief Stores the internal state of the unique
        /// Engine object
        FrameTech::Engine::State m_state;
        /// @brief The graphics pipeline
        FrameTech::Pipeline m_pipeline;
        /// @brief The engine instance
        VkInstance m_graphics_instance;

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
    };

} // namespace FrameTech

#endif // engine_hpp
