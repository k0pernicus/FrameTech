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
#include "engine/inputs/inputs.hpp"
#include "ftstd/timer.h"
#include "gameframework/world.hpp"
#include "project.hpp"
#include <GLFW/glfw3.h>
#include <optional>

#ifdef IMGUI
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#endif

#ifdef NO_AVG_FPS_RECORDS
#define FPS_RECORDS 1
#else
#define FPS_RECORDS 10
#endif

namespace frametech
{
    /// @brief The default height, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_HEIGHT = 1080;
    /// @brief The default width, in pixels, of the window application
    constexpr uint32_t const DEFAULT_WINDOW_WIDTH = 1920;

    /// @brief `Application` handles the entire application / engine.
    /// This class is **not** thread-safe!
    /// Do not execute any preliminar `GetInstance` methods
    /// in threads at first, as `GetInstance` returns a
    /// singleton of `Application`.
    class Application
    {
    private:
        /// @brief Defines the current state of the application
        enum struct State
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
#ifdef IMGUI
        // TODO: add in a proper namespace called 'gui'
        /// @brief ImGui window
        static ImGui_ImplVulkanH_Window m_imgui_app_window;
        /// @brief Setup the ImGui window & API
        void setupImGui();
        /// @brief Upload the ImGui font to avoid rendering error(s)
        ftstd::VResult uploadImGuiFont();
        /// @brief The draw function for the debug tool
        void drawDebugToolImGui();
        /// @brief The draw function for the mesh selector
        void drawMeshSelectionImGui();
        /// @brief Clean the instance(s) of ImGui
        void cleanImGui();
#endif
        /// @brief App window
        GLFWwindow* m_app_window = nullptr;
        /// @brief The windows width size
        int m_app_width = 0;
        /// @brief The windows height size
        int m_app_height = 0;
        /// @brief App title
        const char* m_app_title = nullptr;
        /// @brief Constructor of the Application - should be private as Application is a Singleton
        Application(const char* app_title);
        /// @brief The internal state of the current Application object
        State m_state = State::UNINITIALIZED;
        /// @brief The frame that is being draw
        uint64_t m_current_frame = 1;
        /// @brief The unique instance of the Engine object
        std::unique_ptr<frametech::Engine> m_engine;
        /// @brief Recorded frames to make
        /// FPS stats on latest records
        uint64_t recorded_frames[FPS_RECORDS];
        /// @brief The index to record the current FPS
        /// record
        uint8_t recorded_frames_index = 0;
        /// @brief A private timer
        std::unique_ptr<ftstd::Timer> m_app_timer = nullptr;
        /// @brief The monitor to set / display the application
        frametech::graphics::Monitor m_monitor;
        /// @brief The world, nothing less, nothing more
        frametech::gameframework::World m_world;

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
        ftstd::VResult initWindow();
        /// @brief Initialize the app's engine
        /// @return A boolean value to indicate if the engine has been initialized or not
        bool initEngine();
        /// @brief Initialize the app's engine descriptor sets
        void initDescriptorSets();
        /// @brief Load all assets for the game - should be called
        /// **after** initializing the engine
        ftstd::VResult loadGameAssets() noexcept;
        /// @brief Run the app and wait until the user close it
        void run();
        /// @brief Draw the frame x
        void drawFrame();
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
        /// @brief Returns the current 2D/3D world of the game
        frametech::gameframework::World& getCurrentWorld() noexcept;
        /// @brief Key events job manager
        frametech::engine::inputs::EventHandler m_key_events_handler;
        /// @brief Cursor events job manager
        frametech::engine::inputs::CursorHandler m_cursor_events_handler;
    };

} // namespace frametech

#endif /* application_hpp */
