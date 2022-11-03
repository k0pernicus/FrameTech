//
//  application.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#include "application.hpp"
#include "ftstd/debug_tools.h"
#include "ftstd/timer.h"
#include "project.hpp"

FrameTech::Application* FrameTech::Application::m_instance{nullptr};

FrameTech::Application::Application(const char* app_title)
{
    m_app_title = app_title;
    m_app_timer = std::unique_ptr<Timer>(new Timer());
}

FrameTech::Application::~Application()
{
    clean();
    Log("< Closing the Application object...");
    m_instance = nullptr;
}

GLFWwindow* FrameTech::Application::getWindow() const
{
    return m_app_window;
}

FrameTech::Application* FrameTech::Application::getInstance(const char* app_title)
{
    if (m_instance == nullptr)
    {
        Log("> Instanciating a new Application singleton");
        m_instance = new FrameTech::Application(app_title);
    }
    return m_instance;
}

void FrameTech::Application::clean()
{
    Log("< Cleaning the Application object");
    m_app_title = nullptr;
    m_engine = nullptr;
    glfwDestroyWindow(m_app_window);
    m_app_window = nullptr;
    glfwTerminate();
}

void FrameTech::Application::initWindow()
{
    Log("> Initializing the Application window");
    glfwInit();                                   // Initialize the GLFW library
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context, as we use Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // No resizable option for the window
    // The last parameter in glfwCreateWindow is only for OpenGL - no need to setup it here
    m_app_window = glfwCreateWindow(FrameTech::DEFAULT_WINDOW_WIDTH,
                                    FrameTech::DEFAULT_WINDOW_HEIGHT,
                                    m_app_title,
                                    nullptr,
                                    nullptr);
}

void FrameTech::Application::initEngine()
{
    m_engine = std::unique_ptr<FrameTech::Engine>(FrameTech::Engine::getInstance());
    m_engine->initialize();
}

void FrameTech::Application::forceRendererFPSLimit(uint8_t new_limit)
{
    if (m_FPS_limit == std::nullopt)
        Log("Setting FPS limit to %d", new_limit);
    else if (new_limit > 0)
        Log("Replacing FPS limit from %d to %d", m_FPS_limit, new_limit);
    else
        Log("Disabling FPS limit");
    m_FPS_limit = new_limit > 0 ? std::optional<uint8_t>(new_limit) : std::nullopt;
}

void FrameTech::Application::drawFrame()
{
    const double wait_ms = (m_FPS_limit == std::nullopt) ? 0.0 : (1000 / m_FPS_limit.value());
    if (wait_ms > 0.0)
    {
        uint64_t wait_until_ms = Timer::get_time_limit(wait_ms);
        Log("Drawing frame %d...", m_current_frame);

        // TODO: Draw command

        // Force to pause the rendering thread
        // if (and only if) the time has not come yet
        m_app_timer->block_until(wait_until_ms);
        m_engine->m_render->updateFrameIndex(m_current_frame);
        ++m_current_frame;
        return;
    }
    Log("Drawing frame %d", m_current_frame);
    m_engine->m_render->updateFrameIndex(m_current_frame);
    ++m_current_frame;
}

void FrameTech::Application::run()
{
    switch (m_engine->getState())
    {
        case FrameTech::Engine::State::UNINITIALIZED:
        {
            LogE("< Running the application if the engine is uninitialized is forbidden!");
        }
        case FrameTech::Engine::State::ERROR: // Used by UNINITIALIZED as well
        {
            m_state = FrameTech::Application::State::SHOULD_BE_CLOSED;
        }
        break;
        case FrameTech::Engine::State::INITIALIZED:
        {
            Log("> Application loop...");
            m_FPS_limit.has_value() ? Log("> Application is running at %d FPS", m_FPS_limit.value()) : Log("> Application is running at unlimited frame");
            m_state = FrameTech::Application::State::RUNNING;
            // TODO: Record the duration in the loop
            // TODO: Each second, added the FPS number in recorded_frames
            // TODO: Increase the index : recorded_frames_index = (recorded_frames_index + 1) % FPS_RECORDS;
            while (!glfwWindowShouldClose(m_app_window) && m_state == FrameTech::Application::State::RUNNING)
            {
                glfwPollEvents();
                drawFrame();
            }
            Log("< ...Application loop");
            vkDeviceWaitIdle(m_engine->m_graphics_device.getLogicalDevice());
        }
        break;
    }
    m_state = FrameTech::Application::State::CLOSING;
}

uint64_t FrameTech::Application::getCurrentFrame()
{
    return m_current_frame;
}
