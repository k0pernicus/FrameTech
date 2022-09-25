//
//  engine.hpp
//  FrameTech
//
//  Created by Antonin on 24/09/2022.
//

#pragma once
#ifndef engine_hpp
#define engine_hpp

#include "pipeline.hpp"
#include "vulkan/vulkan.h"
#include <cstdlib>

namespace FrameTech
{

    class Engine
    {

        /// @brief Internal state of the Engine object,
        /// to know if the unique object needs
        /// to be initialized or not
        enum State
        {
            UNINITIALIZED,
            INITIALIZED,
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
        void createGraphicsInstance();
        /// @brief Stores the internal state of the unique
        /// Engine object
        State m_state;
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
        ~Engine();
    };

} // namespace FrameTech

#endif // engine_hpp
