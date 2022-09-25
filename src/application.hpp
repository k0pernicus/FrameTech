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
#include "engine/pipeline.hpp"
#include <GLFW/glfw3.h>
#include <optional>

namespace FrameTech
{

    /// The default height, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_HEIGHT = 580;
    /// The default width, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_WIDTH = 700;

    /// `Application` handles the entire application / engine.
    /// This class is **not** thread-safe!
    /// Do not execute any preliminar `GetInstance` methods
    /// in threads at first, as `GetInstance` returns a
    /// singleton of `Application`.
    class Application
    {
    private:
        /// Defines the current state of the application
        enum State
        {
            /// The default and starting state of the app
            UNINITIALIZED,
            /// The app is running (foreground, background),
            RUNNING,
            /// The app is paused and the draw (or input) events should **not** be listened.
            PAUSED,
            /// The app is not in a correct state (error in treatment for example),
            /// and should be closed
            SHOULD_BE_CLOSED,
            /// Last step: the app is closing
            CLOSING
        };
        static Application* m_instance;
        /// App window
        GLFWwindow* m_app_window = nullptr;
        /// App title
        const char* m_app_title = nullptr;
        /// Constructor of the Application - should be private as Application is a Singleton
        Application(const char* app_title);
        /// The internal state of the current Application object
        State m_state = UNINITIALIZED;
        /// The frame that is being draw
        uint64_t m_current_frame = 1;
        /// The current FPS limit per second to draw
        /// Optional value as the default value is NULL
        std::optional<uint8_t> m_FPS_limit = std::nullopt;
        /// The Engine
        std::unique_ptr<FrameTech::Engine> m_engine;

    public:
        /// Private destructor
        ~Application();
        /// Application should not be cloneable
        Application(Application& other) = delete;
        /// Application should not be assignable
        void operator=(const Application& other) = delete;
        /// The static / single instance of an Application object
        static Application* getInstance(const char* app_title);
        /// Init the clean process to destroy internal instances
        void clean();
        /// Initialize the app window
        void initWindow();
        /// Initialize the app's graphics engine
        void initEngine();
        /// Run the app and wait until the user close it
        void run();
        /// Draw the frame x
        void drawFrame();
        /// Update the internal state of the Application object
        void setState(State new_state);
        /// Returns the internal state of the Application object
        State getState();
        /// Force the renderer to run `FPS_limit` frames per second.
        /// The renderer is not limited by default.
        /// Set `new_limit` to `0` in order to disable the existing limit,
        /// and put it as default state.
        void forceRendererFPSLimit(uint8_t new_limit);
    };

} // namespace FrameTech

#endif /* application_hpp */
