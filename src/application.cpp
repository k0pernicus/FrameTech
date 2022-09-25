//
//  application.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#include "application.hpp"
#include "debug_tools.h"

FrameTech::Application* FrameTech::Application::m_instance{nullptr};

FrameTech::Application::Application(const char* app_title)
{
    m_app_title = app_title;
}

FrameTech::Application::~Application()
{
    clean();
    Log("< Closing the Application object...");
    m_instance = nullptr;
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
    // TODO: pause ?
    // Log("> Drawing frame %d", m_current_frame);
    m_current_frame++;
}

void FrameTech::Application::loop()
{
    Log("> Beginning the Application loop");
    m_state = FrameTech::State::RUNNING;
    // TODO: Record the duration in the loop
    while (!glfwWindowShouldClose(m_app_window) && m_state == FrameTech::State::RUNNING)
    {
        glfwPollEvents();
        drawFrame();
    }
    m_state = FrameTech::State::CLOSING;
    Log("< Ending the Application loop");
}