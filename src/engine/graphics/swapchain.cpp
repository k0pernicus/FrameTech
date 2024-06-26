//
//  swapchain.cpp
//  FrameTech
//
//  Created by Antonin on 29/09/2022.
//

#include "swapchain.hpp"
#include "../../application.hpp"
#include "../../ftstd/debug_tools.h"
#include "../../project.hpp"
#include "../engine.hpp"
#include <GLFW/glfw3.h>

/// @brief 32 bits surface (BGRA, u8 each) in SRGB is
/// prefered for the surface format
constexpr VkFormat PREFERED_SURFACE_FORMAT = VK_FORMAT_B8G8R8A8_SRGB;

/// @brief SRGB non-linear is prefered for the color space - standard color space
constexpr VkColorSpaceKHR PREFERED_COLOR_SPACE_FORMAT = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

/// @brief Internal function to choose automatically a good Surface format
/// from the SwapChainDetails structure
/// @param formats The list of available formats to choose from
/// @return A format that would corresponds to the engine basic needs.
/// If the "perfect" format combination has not been found, then it
/// returns the first element of the array.
static ftstd::Result<VkSurfaceFormatKHR> chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const VkSurfaceFormatKHR format : formats)
    {
        if (format.format == PREFERED_SURFACE_FORMAT && format.colorSpace == PREFERED_COLOR_SPACE_FORMAT)
            return ftstd::Result<VkSurfaceFormatKHR>::Ok(format);
    }
    return ftstd::Result<VkSurfaceFormatKHR>::Ok(formats[0]);
}

/// @brief Internal function to choose automatically a good Presentation Mode
/// from the SwapChainDetails structure.
/// The Presentation Mode represents the actual conditions for showing images
/// to the screen.
/// @param formats The list of available presentation modes to choose from
/// @return A present mode that would corresponds to the engine basic needs
static ftstd::Result<VkPresentModeKHR> choosePresentMode(const std::vector<VkPresentModeKHR>& present_modes)
{
    // In VK_PRESENT_MODE_FIFO_KHR, the swapchain is a queue
    // where the display takes an image from the front of the queue when the display
    // is refreshed and the program inserts rendered images at the back of the queue.
    // If the queue is full then the program has to wait.
    // VK_PRESENT_MODE_FIFO_KHR is the correct way of doing v-sync across all platforms supported by Vulkan **BUT**
    // it needs to be supported everywhere, which does not seem to be the case everywhere.
    // In VK_PRESENT_MODE_IMMEDIATE_KHR, there is no wait and it renders immediately the latest computed image.
    const VkPresentModeKHR prefered_presentation_mode = GAME_APPLICATION_SETTINGS->fps_target.has_value() ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (const VkPresentModeKHR mode : present_modes)
    {
        if (mode == prefered_presentation_mode)
        {
            Log("> Choosed presentation mode with id %d", mode);
            return ftstd::Result<VkPresentModeKHR>::Ok(mode);
        }
    }
    return ftstd::Result<VkPresentModeKHR>::Error((char*)"Our prefered presentation mode is not found");
}

/// @brief Internal function to choose automatically a good capability
/// from the SwapChainDetails structure.
/// The swap extent is the resolution of the swapchain images, which is almost
/// exactly equal to the resolution of the window that we're drawing to (in pixels).
/// @param formats The list of available capabilities to choose from
/// @return A capability that would corresponds to the engine basic needs
static ftstd::Result<VkExtent2D> chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    int window_height, window_width;
    glfwGetFramebufferSize(frametech::Application::getInstance(GAME_APPLICATION_SETTINGS->name.c_str())->getWindow(), &window_width, &window_height);
    VkExtent2D final_extent = {
        static_cast<u32>(window_width),
        static_cast<u32>(window_height),
    };
    final_extent.width = std::clamp(final_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    final_extent.height = std::clamp(final_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return ftstd::Result<VkExtent2D>::Ok(final_extent);
}

frametech::graphics::SwapChain* frametech::graphics::SwapChain::m_instance{nullptr};

frametech::graphics::SwapChain* frametech::graphics::SwapChain::getInstance()
{
    if (nullptr == m_instance)
        m_instance = new frametech::graphics::SwapChain();
    return m_instance;
}

frametech::graphics::SwapChain::SwapChain()
{
}

frametech::graphics::SwapChain::~SwapChain()
{
    Log("< Destroying the swapchain...");
    if (VK_NULL_HANDLE != m_swapchain)
    {
        vkDestroySwapchainKHR(frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
                              m_swapchain,
                              nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    if (nullptr != m_instance)
        m_instance = nullptr;
}

void frametech::graphics::SwapChain::queryDetails()
{
    frametech::graphics::SwapChainSupportDetails support_details{};
    const auto physical_device = frametech::Engine::getInstance()->m_graphics_device.getPhysicalDevice();
    const auto surface = frametech::Engine::getInstance()->m_render->getSurface();

    assert(surface);
    if (nullptr == surface)
        return;

    // Get the capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *surface, &support_details.capabilities);

    // Get the formats
    u32 format_count{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, nullptr);
    if (format_count > 0)
    {
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *surface, &format_count, support_details.formats.data());
    }

    // Get the present modes
    u32 present_modes_count{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_modes_count, nullptr);
    if (present_modes_count > 0)
    {
        support_details.present_modes.resize(present_modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *surface, &present_modes_count, support_details.present_modes.data());
    }

    m_details = support_details;
}

ftstd::VResult frametech::graphics::SwapChain::checkDetails()
{
    const u32 max_frames_count = frametech::Engine::getMaxFramesInFlight();
    if ((m_details.capabilities.minImageCount <= max_frames_count && m_details.capabilities.maxImageCount >= max_frames_count) &&
        (!m_details.formats.empty() && !m_details.present_modes.empty()) &&
        (m_details.capabilities.minImageCount > 0))
    {
        return ftstd::VResult::Ok();
    }
    LogW("The swapchain only supports between %d and %d images (max)", m_details.capabilities.minImageCount, m_details.capabilities.maxImageCount);
    return ftstd::VResult::Error((char*)"The supported images count is incorrect");
}

ftstd::VResult frametech::graphics::SwapChain::create()
{
    // Check that the details are correct
    if (const auto result = checkDetails(); result.IsError())
        return result;

    const auto format_result = chooseFormat(m_details.formats);
    if (format_result.IsError())
        return ftstd::VResult::Error((char*)"Did not found any good format for the swapchain");

    const auto present_mode_result = choosePresentMode(m_details.present_modes);
    if (present_mode_result.IsError())
        return ftstd::VResult::Error((char*)"Did not found any good presentation mode for the swapchain");

    const auto swap_extent_result = chooseSwapExtent(m_details.capabilities);
    if (swap_extent_result.IsError())
        return ftstd::VResult::Error((char*)"Did not found any good extent for the swapchain");

    m_present_mode = present_mode_result.GetValue();
    m_format = format_result.GetValue();
    m_extent = swap_extent_result.GetValue();

    VkSwapchainCreateInfoKHR create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = (*frametech::Engine::getInstance()->m_render->getSurface()),
        .minImageCount = frametech::Engine::getMaxFramesInFlight(),
        .imageFormat = m_format.format,
        .imageColorSpace = m_format.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,                             // Always one (except stereoscopic 3D app)
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // color attachment, use VK_IMAGE_USAGE_TRANSFER_DST_BIT instead
    };
    u32 indices[3] = {
        frametech::Engine::getInstance()->m_graphics_device.m_graphics_queue_family_index,
        frametech::Engine::getInstance()->m_graphics_device.m_presents_queue_family_index,
        frametech::Engine::getInstance()->m_graphics_device.m_transfert_queue_family_index,
    };
    const bool is_exclusive = (indices[0] == indices[1] == indices[2]);
    // TODO: check if the indices car really be equal to each other
    // https://github.com/Overv/VulkanTutorial/issues/233
    assert(!is_exclusive);
    create_info.imageSharingMode = is_exclusive ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    // IMPORTANT NOTE: We **should** be in CONCURRENT mode as the engine is using a Transfert queue
    // that has to be different than the Graphics queue
    assert(create_info.imageSharingMode == VK_SHARING_MODE_CONCURRENT);
    create_info.pQueueFamilyIndices = is_exclusive ? nullptr : indices;
    create_info.queueFamilyIndexCount = is_exclusive ? 0 : sizeof(indices) / sizeof(indices[0]);
    // No transformation
    // TODO: remove for any transformation in the SC
    create_info.preTransform = m_details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = m_present_mode;
    // Enable clipping
    create_info.clipped = VK_TRUE;
    // TODO: change in order to recreate another swapchain (and reference the old one)
    // after a resize
    // WARNING: resize feature does not work
    create_info.oldSwapchain = VK_NULL_HANDLE;

    const auto result = vkCreateSwapchainKHR(frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
                                             &create_info,
                                             nullptr,
                                             &m_swapchain);

    if (result == VK_SUCCESS)
    {
        u32 image_count;
        vkGetSwapchainImagesKHR(
            frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
            m_swapchain,
            &image_count,
            nullptr);
        m_images.resize(image_count);
        vkGetSwapchainImagesKHR(
            frametech::Engine::getInstance()->m_graphics_device.getLogicalDevice(),
            m_swapchain,
            &image_count,
            m_images.data());
        return ftstd::VResult::Ok();
    }

    char* error_msg;
    switch (result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            error_msg = (char*)"out of host memory";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            error_msg = (char*)"out of device memory";
            break;
        case VK_ERROR_DEVICE_LOST:
            error_msg = (char*)"device lost";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            error_msg = (char*)"surface lost";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            error_msg = (char*)"native window in use";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            error_msg = (char*)"initialization failed";
            break;
        default:
            LogE("> vkCreateSwapchainKHR: error 0x%08x", result);
            error_msg = (char*)"undocumented error";
    }
    LogE("> vkCreateSwapchainKHR: %s", error_msg);
    return ftstd::VResult::Error(error_msg);
}

const std::vector<VkImage>& frametech::graphics::SwapChain::getImages() const
{
    return m_images;
}

const VkSurfaceFormatKHR& frametech::graphics::SwapChain::getImageFormat() const
{
    return m_format;
}

const VkExtent2D& frametech::graphics::SwapChain::getExtent() const
{
    return m_extent;
}

const VkSwapchainKHR& frametech::graphics::SwapChain::getSwapchainDevice() const
{
    return m_swapchain;
}
