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

frametech::Application* frametech::Application::m_instance{nullptr};

frametech::Application::Application(const char* app_title)
{
    m_app_title = app_title;
    m_app_timer = std::unique_ptr<ftstd::Timer>(new ftstd::Timer());
}

frametech::Application::~Application()
{
    clean();
    Log("< Closing the Application object...");
    m_instance = nullptr;
}

GLFWwindow* frametech::Application::getWindow() const
{
    return m_app_window;
}

frametech::Application* frametech::Application::getInstance(const char* app_title)
{
    if (m_instance == nullptr)
    {
        Log("> Instanciating a new Application singleton");
        m_instance = new frametech::Application(app_title);
    }
    return m_instance;
}

void frametech::Application::clean()
{
    Log("< Cleaning the Application object");
    m_app_title = nullptr;
    m_engine = nullptr;
    glfwDestroyWindow(m_app_window);
    m_app_window = nullptr;
    glfwTerminate();
}

ftstd::VResult frametech::Application::initWindow()
{
    Log("> Initializing the Application window");
    glfwInit();                                   // Initialize the GLFW library
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context, as we use Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // No resizable option for the window
    // Check for Vulkan support
    if (!glfwVulkanSupported())
    {
        return ftstd::VResult::Error((char*)"Vulkan is not supported");
    }
    // Initialize the monitor object
    if (!m_monitor.foundPrimaryMonitor())
        if (const auto scan_result_code = m_monitor.scanForPrimaryMonitor(); scan_result_code.IsError())
            return ftstd::VResult::Error((char*)"Error getting the primary monitor");

    // The last parameter in glfwCreateWindow is only for OpenGL - no need to setup it here
    m_app_window = glfwCreateWindow(frametech::DEFAULT_WINDOW_WIDTH,
                                    frametech::DEFAULT_WINDOW_HEIGHT,
                                    m_app_title,
                                    nullptr,
                                    nullptr);
    return ftstd::VResult::Ok();
}

void frametech::Application::initEngine()
{
    m_engine = std::unique_ptr<frametech::Engine>(frametech::Engine::getInstance());
    m_engine->initialize();
}

void frametech::Application::forceRendererFPSLimit(uint8_t new_limit)
{
    if (m_FPS_limit == std::nullopt)
        Log("Setting FPS limit to %d", new_limit);
    else if (new_limit > 0)
        Log("Replacing FPS limit from %d to %d", m_FPS_limit, new_limit);
    else
        Log("Disabling FPS limit");
    m_FPS_limit = new_limit > 0 ? std::optional<uint8_t>(new_limit) : std::nullopt;
}

void frametech::Application::drawFrame()
{
    const double wait_ms = (m_FPS_limit == std::nullopt) ? 0.0 : (1000 / m_FPS_limit.value());
    if (wait_ms > 0.0)
    {
        uint64_t wait_until_ms = ftstd::Timer::get_time_limit(wait_ms);
        // Log("Drawing frame %d...", m_current_frame);

        m_engine->m_render->getGraphicsPipeline()->draw();
        m_engine->m_render->getGraphicsPipeline()->present();

        // Force to pause the rendering thread
        // if (and only if) the time has not come yet
        m_app_timer->block_until(wait_until_ms);
        m_engine->m_render->updateFrameIndex(m_current_frame);
        ++m_current_frame;
        return;
    }
    // Log("Drawing frame %d", m_current_frame);
    m_engine->m_render->getGraphicsPipeline()->draw();
    m_engine->m_render->getGraphicsPipeline()->present();
    m_engine->m_render->updateFrameIndex(m_current_frame);
    ++m_current_frame;
}

void frametech::Application::run()
{
    switch (m_engine->getState())
    {
        case frametech::Engine::State::UNINITIALIZED:
        {
            LogE("< Running the application if the engine is uninitialized is forbidden!");
        }
        case frametech::Engine::State::ERROR: // Used by UNINITIALIZED as well
        {
            m_state = frametech::Application::State::SHOULD_BE_CLOSED;
        }
        break;
        case frametech::Engine::State::INITIALIZED:
        {
            Log("> Application loop...");
            m_FPS_limit.has_value() ? Log("> Application is running at %d FPS", m_FPS_limit.value()) : Log("> Application is running at unlimited frame");
            m_state = frametech::Application::State::RUNNING;
            // TODO: Each second, added the FPS number in recorded_frames
            // TODO: Increase the index : recorded_frames_index = (recorded_frames_index + 1) % FPS_RECORDS;
            while (!glfwWindowShouldClose(m_app_window) && m_state == frametech::Application::State::RUNNING)
            {
                glfwPollEvents();
                drawFrame();
            }
            Log("< ...Application loop");
            vkDeviceWaitIdle(m_engine->m_graphics_device.getLogicalDevice());
        }
        break;
    }
    m_state = frametech::Application::State::CLOSING;
}

uint64_t frametech::Application::getCurrentFrame()
{
    return m_current_frame;
}
