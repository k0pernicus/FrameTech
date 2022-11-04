//
//  application.hpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#pragma once
#ifndef application_hpp
#define application_hpp

#include "engine/engine.hpp"
#include "engine/graphics/monitor.hpp"
#include "ftstd/timer.h"
#include "project.hpp"
#include <GLFW/glfw3.h>
#include <optional>

#ifdef NO_AVG_FPS_RECORDS
#define FPS_RECORDS 1
#else
#define FPS_RECORDS 10
#endif

namespace frametech
{

    /// @brief The default height, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_HEIGHT = 580;
    /// @brief The default width, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_WIDTH = 700;

    /// @brief `Application` handles the entire application / engine.
    /// This class is **not** thread-safe!
    /// Do not execute any preliminar `GetInstance` methods
    /// in threads at first, as `GetInstance` returns a
    /// singleton of `Application`.
    class Application
    {
    private:
        /// @brief Defines the current state of the application
        enum State
        {
            /// @brief The default and starting state of the app
            UNINITIALIZED,
            /// @brief The app is running (foreground, background),
            RUNNING,
            /// @brief The app is paused and the draw (or input) events should **not** be listened.
            PAUSED,
            /// @brief The app is not in a correct state (error in treatment for example),
            /// and should be closed
            SHOULD_BE_CLOSED,
            /// @brief The last state of the internal states machine
            CLOSING
        };
        static Application* m_instance;
        /// @brief App window
        GLFWwindow* m_app_window = nullptr;
        /// @brief App title
        const char* m_app_title = nullptr;
        /// @brief Constructor of the Application - should be private as Application is a Singleton
        Application(const char* app_title);
        /// @brief The internal state of the current Application object
        State m_state = UNINITIALIZED;
        /// @brief The frame that is being draw
        uint64_t m_current_frame = 1;
        /// @brief The current FPS limit per second to draw
        /// Optional value as the default value is NULL
        std::optional<uint8_t> m_FPS_limit = Project::APPLICATION_FPS_LIMIT;
        /// @brief The unique instance of the Engine object
        std::unique_ptr<frametech::Engine> m_engine;
        /// @brief Recorded frames to make
        /// FPS stats on latest records
        uint8_t recorded_frames[FPS_RECORDS];
        /// @brief The index to record the current FPS
        /// record
        uint8_t recorded_frames_index = 0;
        /// @brief A private timer
        std::unique_ptr<Timer> m_app_timer = nullptr;
        /// @brief The monitor to set / display the application
        frametech::graphics::Monitor m_monitor;

    public:
        /// @brief Private destructor
        ~Application();
        /// @brief Application should not be cloneable
        Application(Application& other) = delete;
        /// @brief Application should not be assignable
        void operator=(const Application& other) = delete;
        /// @brief The static / single instance of an Application object
        /// @param app_title The application title / name
        static Application* getInstance(const char* app_title);
        /// @brief Init the clean process to destroy internal instances
        void clean();
        /// @brief Initialize the app window
        void initWindow();
        /// @brief Initialize the app's graphics engine
        void initEngine();
        /// @brief Run the app and wait until the user close it
        void run();
        /// @brief Draw the frame x
        void drawFrame();
        /// @brief Update the internal state of the Application object
        /// @param new_state The new state of the object
        void setState(State new_state);
        /// @brief Returns a pointer to the Window instance
        GLFWwindow* getWindow() const;
        /// @brief Returns the internal state of the Application object
        State getState();
        /// @brief Force the renderer to run `FPS_limit` frames per second.
        /// The renderer is not limited by default.
        /// @param new_limit Set `new_limit` to `0` in order to disable the existing limit,
        /// and put it as default state.
        void forceRendererFPSLimit(uint8_t new_limit);
        /// @brief Returns the current frame of the application
        uint64_t getCurrentFrame();
    };

} // namespace frametech

#endif /* application_hpp */
