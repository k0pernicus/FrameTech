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

FrameTech::Graphics::Render* FrameTech::Graphics::Render::m_instance{nullptr};

FrameTech::Graphics::Render::Render() {}

FrameTech::Graphics::Render::~Render()
{
    if (m_surface)
    {
        Log("< Destroying the Window surface...");
        vkDestroySurfaceKHR(FrameTech::Engine::getInstance()->m_graphics_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_image_views.size() > 0)
    {
        Log("< Destroying the image views...");
        for (auto image_view : m_image_views)
            vkDestroyImageView(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(), image_view, nullptr);
        m_image_views.clear();
    }
    m_instance = nullptr;
}

FrameTech::Graphics::Render* FrameTech::Graphics::Render::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Render();
    }
    return m_instance;
}

VResult FrameTech::Graphics::Render::createSurface()
{
    const auto window_surface_result = glfwCreateWindowSurface(
        FrameTech::Engine::getInstance()->m_graphics_instance,
        FrameTech::Application::getInstance(Project::APPLICATION_NAME)->getWindow(),
        nullptr,
        &m_surface);
    if (window_surface_result == VK_SUCCESS)
    {
        return VResult::Ok();
    }
    return VResult::Error((char*)"failed to create a window surface");
}

VkSurfaceKHR* FrameTech::Graphics::Render::getSurface()
{
    return &m_surface;
}
