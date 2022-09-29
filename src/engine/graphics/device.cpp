//
//  device.cpp
//  FrameTech
//
//  Created by Antonin on 26/09/2022.
//

#include "device.hpp"
#include "../../debug_tools.h"
#include "../../result.h"
#include "../engine.hpp"
#include "render.hpp"
#include <vector>

#ifdef DEBUG
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> REQUIRED_LAYERS_TO_CHECK = {
    "VK_KHR_portability_subset",
};
#else
const std::vector<const char*> VALIDATION_LAYERS = {};
const std::vector<const char*> REQUIRED_LAYERS_TO_CHECK = {
    "VK_KHR_portability_subset",
};
#endif

/// @brief The name of the Apple M1 chip, which is the
/// only exception choosing a physical device
const char* APPLE_M1_NAME = (char*)"Apple M1";

/// @brief Boolean flag to know if the physical graphical device
/// needs to support geometry shaders
#define NEEDS_GEOMETRY_SHADER 0

static bool isDeviceSuitable(const VkPhysicalDevice& device)
{
    if (device == nullptr)
    {
        LogE("> cannot get properties if device is NULL");
        return false;
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    Log("> checking device '%s' (with ID '%d')", properties.deviceName, properties.deviceID);

    const bool is_apple_silicon = strcmp(properties.deviceName, APPLE_M1_NAME) == 0;
    Log("\t* is Apple Silicon? %s", is_apple_silicon ? "true!" : "false...");

    const bool is_discrete_gpu = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    Log("\t* is discrete gpu? %s", is_discrete_gpu ? "true!" : "false...");

#if defined(NEEDS_GEOMETRY_SHADER) && NEEDS_GEOMETRY_SHADER == 1
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    const bool supports_geometry_shader = features.geometryShader;
    Log("\t* supports geometry shader? %s", supports_geometry_shader ? "true!" : "false...");

    return is_apple_silicon || (is_discrete_gpu && supports_geometry_shader);
#endif

    return is_apple_silicon || is_discrete_gpu;
}

void FrameTech::Graphics::Device::Destroy()
{
    if (m_logical_device)
    {
        vkDestroyDevice(m_logical_device, nullptr);
        m_logical_device = VK_NULL_HANDLE;
    }
}

FrameTech::Graphics::Device::~Device()
{
    Destroy();
    m_queue_support.clear();
    m_physical_device = VK_NULL_HANDLE;
    m_graphics_queue = VK_NULL_HANDLE;
    m_presents_queue = VK_NULL_HANDLE;
    Log("< Destroying the Physical and Logical device instance...");
}

uint32_t FrameTech::Graphics::Device::getNumberDevices() const
{
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(FrameTech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    return device_count;
}

Result<int> FrameTech::Graphics::Device::listDevices()
{
    Result<int> result;
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(FrameTech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    if (device_count == 0)
    {
        result.Error((char*)"no supported physical device");
        return result;
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(FrameTech::Engine::getInstance()->m_graphics_instance, &device_count, devices.data());
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            m_physical_device = device;
            Log("\t... is suitable!");
            result.Ok(RESULT_OK);
            return result;
        }
        Log("\t... is **not** suitable!");
    }
    result.Error((char*)"no suitable physical device");
    return result;
}

bool FrameTech::Graphics::Device::isInitialized() const
{
    return m_physical_device != VK_NULL_HANDLE;
}

Result<uint32_t> FrameTech::Graphics::Device::getQueueFamilies()
{
    Result<uint32_t> result;
    assert(isInitialized());
    if (!isInitialized())
    {
        result.Error((char*)"The physical device has not been setup");
        return result;
    }
    uint32_t queue_families_number = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_families_number, nullptr);
    result.Ok(queue_families_number);
    if (queue_families_number == 0)
    {
        Log("No queue families for the selected physical device");
        return result;
    }
    std::vector<VkQueueFamilyProperties> found_queue_families(queue_families_number);
    m_queue_support.resize((size_t)queue_families_number);
    m_queue_states.resize((size_t)queue_families_number);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_families_number, found_queue_families.data());

    // Check if there is at least one queue that supports graphics
    for (uint32_t i = 0; i < queue_families_number; i++)
    {
        Log("\t> checking queue family %d", i);

        // Supports graphics queue?
        const bool graphics_supported = (found_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
        Log("\t\t* graphics feature supported? %s", graphics_supported ? "true!" : "false...");
        m_queue_support[i] = graphics_supported ? SupportFeatures::GRAPHICS : SupportFeatures::NOONE;

        // Supports present queue?
        VkBool32 present_supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            m_physical_device,
            i,
            *FrameTech::Graphics::Render::getInstance()->getSurface(),
            &present_supported);
        Log("\t\t* present feature supported? %s", present_supported ? "true!" : "false...");
        m_queue_support[i] |= present_supported ? SupportFeatures::PRESENTS : SupportFeatures::NOONE;
        m_queue_states[i] = m_queue_support[i] == SupportFeatures::NOONE ? QueueState::UNSUPPORTED : QueueState::READY;
    }

    result.Ok(queue_families_number);
    return result;
}

Result<int> FrameTech::Graphics::Device::createLogicalDevice()
{
    assert(m_physical_device);
    Result<int> result;
    if (m_physical_device == VK_NULL_HANDLE)
    {
        result.Error((char*)"The physical device has not been setup");
        return result;
    }
    uint32_t first_index = 0;
    for (int i = 0; i < m_queue_support.size(); i++)
    {
        first_index = i;
        if (m_queue_support[i] != SupportFeatures::NOONE)
            break;
    }
    if (first_index >= m_queue_support.size())
    {
        result.Error((char*)"No any READY queue for the physical device");
        return result;
    }
    // Set the first indexed queue as USED
    m_queue_states[first_index] = QueueState::USED;
    // Create the Queue information
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = first_index; // The first READY queue
    queue_create_info.queueCount = 1;                 // Enable one queue - low-overhead calls using multithreading
    // Influences the scheduling of command buffer execution (1.0 is the max priority value)
    // Required, even for a single queue
    float queue_priority = 1.0;
    queue_create_info.pQueuePriorities = &queue_priority;

    // Specify GRAPHICS feature - set everyone
    // to VK_FALSE for the moment
    VkPhysicalDeviceFeatures device_features{};

    // Initializes the logical device
    VkDeviceCreateInfo logical_device_create_info{};
    logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_device_create_info.pQueueCreateInfos = &queue_create_info;
    logical_device_create_info.queueCreateInfoCount = 1;
    logical_device_create_info.pEnabledFeatures = &device_features;
    logical_device_create_info.enabledExtensionCount = REQUIRED_LAYERS_TO_CHECK.size();
    logical_device_create_info.ppEnabledExtensionNames = REQUIRED_LAYERS_TO_CHECK.data();
    if (const auto result_status = vkCreateDevice(m_physical_device, &logical_device_create_info, nullptr, &m_logical_device); result_status != VK_SUCCESS)
    {
        char* error_msg;
        switch (result_status)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                error_msg = (char*)"out of host memory";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                error_msg = (char*)"out of device memory";
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                error_msg = (char*)"initialization memory";
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                error_msg = (char*)"extension not present";
                break;
            case VK_ERROR_FEATURE_NOT_PRESENT:
                error_msg = (char*)"feature not present";
                break;
            case VK_ERROR_TOO_MANY_OBJECTS:
                error_msg = (char*)"too many objects";
                break;
            case VK_ERROR_DEVICE_LOST:
                error_msg = (char*)"device lost";
                break;
            default:
                Log("> vkCreateInstance: error 0x%08x", result_status);
                error_msg = (char*)"undocumented error";
        }
        LogE("> vkCreateInstance: %s", error_msg);
        result.Error((char*)"Cannot create the logical device");
        return result;
    }
    Log("> Logical device has been created");
    // Let's use 0 by default for queue index as we create one queue per logical device
    vkGetDeviceQueue(m_logical_device, first_index, 0, &m_graphics_queue);
    result.Ok(RESULT_OK);
    return result;
}
