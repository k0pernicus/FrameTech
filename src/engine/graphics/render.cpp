//
//  render.cpp
//  FrameTech
//
//  Created by Antonin on 28/09/2022.
//

#include "render.hpp"
#include "../../application.hpp"
#include "../../debug_tools.h"
#include "../../project.hpp"
#include "../../result.h"
#include "../engine.hpp"

// #define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

FrameTech::Graphics::Render* FrameTech::Graphics::Render::m_render_instance{nullptr};

FrameTech::Graphics::Render::Render() {}

FrameTech::Graphics::Render::~Render()
{
    if (m_surface)
    {
        Log("< Destroying the Window surface...");
        vkDestroySurfaceKHR(FrameTech::Engine::getInstance()->m_graphics_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    m_render_instance = nullptr;
}

FrameTech::Graphics::Render* FrameTech::Graphics::Render::getInstance()
{
    if (m_render_instance == nullptr)
    {
        m_render_instance = new Render();
    }
    return m_render_instance;
}

Result<int> FrameTech::Graphics::Render::createSurface()
{
    Result<int> result;
    const auto window_surface_result = glfwCreateWindowSurface(
        FrameTech::Engine::getInstance()->m_graphics_instance,
        FrameTech::Application::getInstance(Project::APPLICATION_NAME)->getWindow(),
        nullptr,
        &m_surface);
    if (window_surface_result == VK_SUCCESS)
    {
        result.Ok(RESULT_OK);
        return result;
    }
    result.Error((char*)"failed to create a window surface");
    return result;
}

VkSurfaceKHR* FrameTech::Graphics::Render::getSurface()
{
    return &m_surface;
}
