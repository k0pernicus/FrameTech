//
//  device.cpp
//  FrameTech
//
//  Created by Antonin on 26/09/2022.
//

#include "device.hpp"
#include "../debug_tools.h"
#include "../result.h"
#include "engine.hpp"
#include <vector>

/// @brief The name of the Apple M1 chip, which is the
/// only exception choosing a physical device
const char* APPLE_M1_NAME = (char*)"Apple M1";

static bool isDeviceSuitable(const VkPhysicalDevice& device)
{
    if (device == nullptr)
    {
        LogE("> cannot get properties if device is NULL");
        return false;
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    Log("> checking device '%s' (with ID '%d')", properties.deviceName, properties.deviceID);

    const bool is_discrete_gpu = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    Log("\t* is discrete gpu? %s", is_discrete_gpu ? "true!" : "false...");

    const bool supports_geometry_shader = features.geometryShader;
    Log("\t* supports geometry shader? %s", supports_geometry_shader ? "true!" : "false...");

    const bool is_apple_silicon = strcmp(properties.deviceName, APPLE_M1_NAME) == 0;
    Log("\t* is Apple Silicon? %s", is_apple_silicon ? "true!" : "false...");

    return is_apple_silicon || (is_discrete_gpu && supports_geometry_shader);
}

FrameTech::Device::~Device()
{
    m_physical_device = VK_NULL_HANDLE;
}

uint32_t FrameTech::Device::getNumberDevices() const
{
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(FrameTech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    return device_count;
}

Result<int> FrameTech::Device::listDevices()
{
    Result<int> result;
    uint32_t device_count{};
    vkEnumeratePhysicalDevices(FrameTech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    if (device_count == 0)
    {
        result.Error(RESULT_ERROR, "no supported physical device");
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
    result.Error(RESULT_ERROR, "no suitable physical device");
    return result;
}

inline bool FrameTech::Device::isInitialized() const
{
    return m_physical_device != VK_NULL_HANDLE;
}

Result<uint32_t> FrameTech::Device::getQueueFamilies()
{
    Result<uint32_t> result;
    assert(isInitialized());
    if (!isInitialized())
    {
        result.Error(RESULT_ERROR, "No physical device");
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
    m_queues_states.resize((size_t)queue_families_number);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_families_number, found_queue_families.data());

    // Check if there is at least one queue that supports graphics
    for (uint32_t i = 0; i < queue_families_number; i++)
    {
        Log("\t> checking queue family %d", i);
        const bool is_supported = (found_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
        Log("\t\t* is supported? %s", is_supported ? "true!" : "false...");
        m_queues_states[i] = is_supported ? QueueState::READY : QueueState::UNSUPPORTED;
    }
    result.Ok(queue_families_number);
    return result;
}
