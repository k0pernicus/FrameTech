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

VResult FrameTech::Graphics::Render::createImageViews()
{
    const auto swapchain_images = FrameTech::Engine::getInstance()->m_swapchain->getImages();
    const size_t nb_swapchain_images = swapchain_images.size();
    m_image_views.resize(nb_swapchain_images);
    Log("> %d image views to create (for the render object)", nb_swapchain_images);
    for (size_t i = 0; i < nb_swapchain_images; i++)
    {
        // Create a VkImageView for each VkImage from the swapchain
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = swapchain_images[i];
        // How the image data should be interpreted
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // could be 1D / 2D / 3D texture, or cube maps
        image_view_create_info.format = FrameTech::Engine::getInstance()->m_swapchain->getImageFormat().format;
        // Default mapping
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        const auto image_view_result = vkCreateImageView(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
                                                         &image_view_create_info,
                                                         nullptr,
                                                         &m_image_views[i]);
        if (image_view_result == VK_SUCCESS)
        {
            Log("\t* image view %d... ok!", i);
            continue;
        }

        char* error_msg;
        switch (image_view_result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                error_msg = (char*)"out of host memory";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                error_msg = (char*)"out of device memory";
                break;
            default:
                LogE("> vkCreateImageView: unknown error 0x%08x", image_view_result);
                error_msg = (char*)"undocumented error";
                break;
        }
        LogE("Error creating the image view %d: %s", i, error_msg);
        return VResult::Error(error_msg);
    }
    return VResult::Ok();
}
