//
//  swapchain.cpp
//  FrameTech
//
//  Created by Antonin on 29/09/2022.
//

#include "swapchain.hpp"
#include "../../debug_tools.h"
#include "../engine.hpp"

FrameTech::Graphics::SwapChain* FrameTech::Graphics::SwapChain::m_instance{nullptr};

FrameTech::Graphics::SwapChain* FrameTech::Graphics::SwapChain::getInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new FrameTech::Graphics::SwapChain();
    }
    return m_instance;
}

FrameTech::Graphics::SwapChain::SwapChain()
{
}

FrameTech::Graphics::SwapChain::~SwapChain()
{
    Log("< Destroying the swapchain...");
    if (m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(FrameTech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
                              m_swapchain,
                              nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    if (m_instance != nullptr)
        m_instance = nullptr;
}

void FrameTech::Graphics::SwapChain::queryDetails()
{
    FrameTech::Graphics::SwapChainSupportDetails support_details{};
    const auto physical_device = FrameTech::Engine::getInstance()->m_graphics_device.getPhysicalDevice();
    const auto surface = FrameTech::Engine::getInstance()->m_render->getSurface();

    assert(surface);
    if (surface == nullptr)
        return;

    // Get the capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *surface, &support_details.capabilities);

    // Get the formats
    uint32_t format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, nullptr);
    if (format_count > 0)
    {
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, support_details.formats.data());
    }

    // Get the present modes
    uint32_t present_modes_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_modes_count, nullptr);
    if (present_modes_count > 0)
    {
        support_details.present_modes.resize(present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_modes_count, support_details.present_modes.data());
    }

    m_details = support_details;
}

Result<int> FrameTech::Graphics::SwapChain::checkDetails()
{
    if ((m_details.capabilities.minImageCount >= MAX_BUFFERS && m_details.capabilities.maxImageCount <= MAX_BUFFERS) &&
        (!m_details.formats.empty() && m_details.present_modes.empty()) &&
        (m_details.capabilities.minImageCount > 0))
        return Result<int>::Ok(RESULT_OK);
    LogW("< The swapchain only supports between %d and %d images (max)", m_details.capabilities.minImageCount, m_details.capabilities.maxImageCount);
    return Result<int>::Error((char*)"The supported images count is incorrect");
}

Result<int> FrameTech::Graphics::SwapChain::createSwapChain()
{
    WARN_RT_UNIMPLEMENTED;
    return Result<int>::Error((char*)"Not implemented");
}
