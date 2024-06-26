//
//  device.cpp
//  FrameTech
//
//  Created by Antonin on 26/09/2022.
//

#include "device.hpp"
#include "../../ftstd/debug_tools.h"
#include "../engine.hpp"
#include "render.hpp"

#ifdef DEBUG
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> REQUIRED_EXTENSIONS = {
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
    "VK_KHR_swapchain",
};
#else
const std::vector<const char*> VALIDATION_LAYERS = {};
const std::vector<const char*> REQUIRED_EXTENSIONS = {
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
    "VK_KHR_swapchain",
};
#endif

/// @brief Boolean flag to know if the physical graphical device
/// needs to support geometry shaders
#define NEEDS_GEOMETRY_SHADER 0

static bool isDeviceSuitable(const VkPhysicalDevice& device, const frametech::graphics::DeviceSupportsOptions& options)
{
    if (nullptr == device)
    {
        LogE("> cannot get properties if device is NULL");
        return false;
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    Log("> Checking device '%s' (with ID '%d')", properties.deviceName, properties.deviceID);
    if (!options.supports_device_names.empty()) {
        for (const std::string& device_name: options.supports_device_names) {
            if (strcmp(properties.deviceName, device_name.c_str()) == 0) {
                Log("\t* found device with name '%s'", properties.deviceName);
                return true;
            }
        }
    }
    const bool is_discrete_gpu = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    Log("\t* is discrete gpu? %s", is_discrete_gpu ? "true!" : "false...");

    const bool is_integrated_gpu = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    Log("\t* is integrated gpu? %s", is_integrated_gpu ? "true!" : "false...");

#if defined(NEEDS_GEOMETRY_SHADER) && NEEDS_GEOMETRY_SHADER == 1
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    const bool supports_geometry_shader = features.geometryShader;
    Log("\t* supports geometry shader? %s", supports_geometry_shader ? "true!" : "false...");

    return is_apple_silicon || (is_discrete_gpu && supports_geometry_shader);
#endif

    return (options.supports_integrated_graphics_device && is_integrated_gpu) || is_discrete_gpu;
}

static void listAvailableExtensions(const VkPhysicalDevice& physical_device)
{
    u32 available_extensions_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extensions_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions;
    available_extensions.resize(available_extensions_count);

    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extensions_count, available_extensions.data());

    Log("%d available extensions for this system", available_extensions_count);
    for (int i = 0; i < available_extensions_count; i++)
        Log("\t* %s", available_extensions[i].extensionName);

    for (const auto& required_extension : REQUIRED_EXTENSIONS)
    {
        bool extension_found = false;
        for (int i = 0; i < available_extensions_count; i++)
        {
            const auto extension_name = available_extensions[i].extensionName;
            if (strcmp(extension_name, required_extension) == 0)
            {
                extension_found = true;
                Log("> Using extension '%s'...", extension_name);
                break;
            }
        }
        if (!extension_found)
        {
            LogW("> Layer '%s' has not been found!", required_extension);
            LogW("> This may throw an 'VK_ERROR_EXTENSION_NOT_PRESENT' error creating the Vulkan instance");
        }
    }
}

void frametech::graphics::Device::Destroy()
{
    if (VK_NULL_HANDLE != m_logical_device)
    {
        vkDestroyDevice(m_logical_device, nullptr);
        m_logical_device = VK_NULL_HANDLE;
    }
}

frametech::graphics::Device::~Device()
{
    Destroy();
    m_queue_support.clear();
    m_physical_device = VK_NULL_HANDLE;
    m_graphics_queue = VK_NULL_HANDLE;
    m_presents_queue = VK_NULL_HANDLE;
    Log("< Destroying the physical, and logical, devices...");
}

u32 frametech::graphics::Device::getNumberDevices() const
{
    u32 device_count{};
    vkEnumeratePhysicalDevices(frametech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    return device_count;
}

ftstd::VResult frametech::graphics::Device::listDevices(const frametech::graphics::DeviceSupportsOptions& options)
{
    u32 device_count{};
    vkEnumeratePhysicalDevices(frametech::Engine::getInstance()->m_graphics_instance, &device_count, nullptr);
    if (device_count == 0)
    {

        return ftstd::VResult::Error((char*)"no supported physical device");
    }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(frametech::Engine::getInstance()->m_graphics_instance, &device_count, devices.data());
    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, options))
        {
            m_physical_device = device;
            Log("\t... is suitable!");
            listAvailableExtensions(device);
            return ftstd::VResult::Ok();
        }
        Log("\t... is **not** suitable!");
    }
    return ftstd::VResult::Error((char*)"no suitable physical device");
}

bool frametech::graphics::Device::isInitialized() const
{
    return VK_NULL_HANDLE != m_physical_device;
}

ftstd::Result<u32> frametech::graphics::Device::getQueueFamilies()
{
    assert(isInitialized());
    if (!isInitialized())
    {
        return ftstd::Result<u32>::Error((char*)"The physical device has not been setup");
    }
    u32 total_queue_families = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &total_queue_families, nullptr);
    if (total_queue_families == 0)
    {
        Log("No queue families for the selected physical device");
        return ftstd::Result<u32>::Ok(total_queue_families);
    }
    std::vector<VkQueueFamilyProperties> found_queue_families(total_queue_families);
    m_queue_support.resize((int)total_queue_families);
    m_queue_states.resize((int)total_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &total_queue_families, found_queue_families.data());

    // Constructs the set of internal queues
    for (u32 i = 0; i < total_queue_families; i++)
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
            *frametech::graphics::Render::getInstance()->getSurface(),
            &present_supported);
        Log("\t\t* present feature supported? %s", present_supported ? "true!" : "false...");
        m_queue_support[i] |= present_supported ? SupportFeatures::PRESENTS : SupportFeatures::NOONE;

        // Supports transfert queue?
        const bool transfert_supported = (found_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT);
        Log("\t\t* transfert feature supported? %s", transfert_supported ? "true!" : "false...");
        m_queue_support[i] |= transfert_supported ? SupportFeatures::TRANSFERT : SupportFeatures::NOONE;

        // Save the state
        m_queue_states[i] = m_queue_support[i] == SupportFeatures::NOONE ? QueueState::UNSUPPORTED : QueueState::READY;
    }

    bool supports_graphics = false;
    bool supports_present = false;
    bool supports_transfert = false;
    for (int i = 0; i < total_queue_families; i++)
    {
        supports_graphics = (m_queue_support[i] & SupportFeatures::GRAPHICS) != 0 || supports_graphics;
        supports_present = (m_queue_support[i] & SupportFeatures::PRESENTS) != 0 || supports_present;
        supports_transfert = (m_queue_support[i] & SupportFeatures::TRANSFERT) != 0 || supports_transfert;
        if (supports_graphics && supports_present && supports_transfert)
            break;
    }
    if (!supports_graphics || !supports_present || !supports_transfert)
    {
        return ftstd::Result<u32>::Error((char*)"did not found any queue that support our requirements");
    }

    return ftstd::Result<u32>::Ok(total_queue_families);
}

ftstd::VResult frametech::graphics::Device::createLogicalDevice()
{
    assert(m_physical_device);
    if (VK_NULL_HANDLE == m_physical_device)
    {
        return ftstd::VResult::Error((char*)"The physical device has not been setup");
    }

    const SupportFeatures supported_flags[3] = {
        SupportFeatures::GRAPHICS,
        SupportFeatures::PRESENTS,
        SupportFeatures::TRANSFERT};
    std::vector<VkDeviceQueueCreateInfo> queues(sizeof(supported_flags) / sizeof(supported_flags[0]));
    // TODO: make Vulkan uses the same queue for GRAPHICS and PRESENTS,
    // and avoid this trick
    int took_indices[3] = {-1, -1, -1};
    // Influences the scheduling of command buffer execution (1.0 is the max priority value)
    // Required, even for a single queue
    f32 queue_priority = 1.0f;

    int queue_index = 0;
    // TODO: initialize and set the present queue here
    for (const SupportFeatures supported_flag : supported_flags)
    {
        u32 first_index = 0;
        for (int i = 0; i < m_queue_support.size(); i++)
        {
            first_index = i;
            // If the current index is taken, go next, as
            // we need to specify different queue indices to
            // different families in Vulkan
            if (std::find(std::begin(took_indices), std::end(took_indices), first_index) != std::end(took_indices))
                continue;
            if (m_queue_support[i] & supported_flag)
                break;
        }
        took_indices[queue_index] = first_index;
        if (first_index >= m_queue_support.size())
        {
            return ftstd::VResult::Error((char*)"No any READY queue for the physical device");
        }
        // Set the first indexed queue as USED
        m_queue_states[first_index] = QueueState::USED;
        // Create the Queue information
        VkDeviceQueueCreateInfo queue_create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = first_index, // The first READY queue
            .queueCount = 1,                 // Enable one queue - low-overhead calls using multithreading
            .pQueuePriorities = &queue_priority,
        };
        queues[queue_index++] = queue_create_info;
        switch (supported_flag)
        {
            case SupportFeatures::GRAPHICS:
                m_graphics_queue_family_index = first_index;
                Log("> Graphics family queue index is %d", first_index);
                break;
            case SupportFeatures::PRESENTS:
                m_presents_queue_family_index = first_index;
                Log("> Presents family queue index is %d", first_index);
                break;
            case SupportFeatures::TRANSFERT:
                m_transfert_queue_family_index = first_index;
                Log("> Transfert family queue index is %d", first_index);
                break;
            default:
                LogE("unknown flag for SupportFeatures: %d", supported_flag);
                return ftstd::VResult::Error((char*)"found unknown flag for SupportFeatures");
        }
    }

    // Specify GRAPHICS feature
    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    // Initializes the logical device
    VkDeviceCreateInfo logical_device_create_info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<u32>(queues.size()),
        .pQueueCreateInfos = queues.data(),
        .enabledExtensionCount = static_cast<u32>(REQUIRED_EXTENSIONS.size()),
        .ppEnabledExtensionNames = REQUIRED_EXTENSIONS.data(),
        .pEnabledFeatures = &device_features,
    };
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
                Log("> vkCreateDevice: error 0x%08x", result_status);
                error_msg = (char*)"undocumented error";
        }
        LogE("> vkCreateDevice: %s", error_msg);
        return ftstd::VResult::Error((char*)"Cannot create the logical device");
    }
    Log("> Logical device has been created");

    // Create the graphics queue
    // Let's use 0 by default for queue index as we create one queue per logical device
    vkGetDeviceQueue(m_logical_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

    // Create the present queue
    // Let's use 0 by default for queue index as we create one queue per logical device
    vkGetDeviceQueue(m_logical_device, m_presents_queue_family_index, 0, &m_presents_queue);

    // Create the transfert queue, which is used by buffer copy commands (to move the data
    // from the staging buffer to the vertex buffer)
    // Let's use 0 by default for queue index as we create one queue per logical device
    vkGetDeviceQueue(m_logical_device, m_transfert_queue_family_index, 0, &m_transfert_queue);

    return ftstd::VResult::Ok();
}

const VkDevice& frametech::graphics::Device::getLogicalDevice() const
{
    return m_logical_device;
}

VkPhysicalDevice frametech::graphics::Device::getPhysicalDevice() const
{
    return m_physical_device;
}

VkQueue& frametech::graphics::Device::getGraphicsQueue()
{
    return m_graphics_queue;
}

VkQueue& frametech::graphics::Device::getPresentsQueue()
{
    return m_presents_queue;
}

VkQueue& frametech::graphics::Device::getTransfertQueue()
{
    return m_transfert_queue;
}
