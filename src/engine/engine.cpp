//
//  engine.cpp
//  FrameTech
//
//  Created by Antonin on 24/09/2022.
//

#include "engine.hpp"
#include "../application.hpp"
#include "../debug_tools.h"
#include "../project.hpp"
#include "../result.h"
#include "project.hpp"
#include <vulkan/vulkan.h>

#ifdef DEBUG
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> REQUIRED_LAYERS_TO_CHECK = {
    "VK_EXT_debug_utils",
    "VK_KHR_portability_subset",
};
#else
const std::vector<const char*> VALIDATION_LAYERS = {};
const std::vector<const char*> REQUIRED_LAYERS_TO_CHECK = {
    "VK_KHR_portability_subset",
};
#endif

static VkApplicationInfo createApplicationInfo()
{
    // The instance is the connection between the application
    // and the Vulkan library
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = Project::APPLICATION_NAME;
    application_info.applicationVersion = VK_MAKE_VERSION(
        Project::APPLICATION_VERSION_MAJOR_NUMBER,
        Project::APPLICATION_VERSION_MINOR_NUMBER,
        Project::APPLICATION_VERSION_BUGFIX_NUMBER);
    application_info.pEngineName = Project::ENGINE_NAME;
    application_info.engineVersion = VK_MAKE_VERSION(
        Project::ENGINE_VERSION_MAJOR_NUMBER,
        Project::ENGINE_VERSION_MINOR_NUMBER,
        Project::ENGINE_VERSION_BUGFIX_NUMBER);
    application_info.apiVersion = VK_API_VERSION_1_3;
    return application_info;
}

static void getRequiredExtensions(const char*** extension_names, uint32_t* extension_count)
{
    // Platform specific additions
    (*extension_names) = glfwGetRequiredInstanceExtensions(extension_count);
    if ((*extension_count) == 0)
    {
        // Is this really an error?
        LogE("> No required extension found...");
        return;
    }
    // List the system required extensions
    Log("> %d required extension(s):", *extension_count);
    for (int i = 0; i < (*extension_count); i++)
        Log("\t* %d -> %s", i, (*extension_names)[i]);
    // Look for a required extension not present as
    // system wide
    for (const auto& required_layer : REQUIRED_LAYERS_TO_CHECK)
    {
        bool layer_found = false;
        for (int i = 0; i < (*extension_count); i++)
        {
            const auto extension_name = (*extension_names)[i];
            if (strcmp(extension_name, required_layer) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            LogW("> Layer '%s' has not been found!", required_layer);
            LogW("> This may throw an 'VK_ERROR_EXTENSION_NOT_PRESENT' error creating the Vulkan instance");
        }
    }
}

static void listSupportedExtensions()
{
    uint32_t supported_extension_count{};
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, nullptr);
    if (supported_extension_count == 0)
    {
        Log("> No supported extension found...");
        return;
    }
    std::vector<VkExtensionProperties> supported_extensions(supported_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &supported_extension_count, supported_extensions.data());
    Log("> %d supported extension(s):", supported_extension_count);
    for (int i = 0; i < supported_extension_count; i++)
        Log("\t* %d -> %s", i, supported_extensions[i].extensionName);
}

FrameTech::Engine* FrameTech::Engine::m_instance{nullptr};

FrameTech::Engine::Engine()
{
    m_state = UNINITIALIZED;
}

FrameTech::Engine::~Engine()
{
    Log("< Closing the Engine object...");
    m_render->~Render();
    m_physical_device.Destroy();
    if (m_graphics_instance)
        vkDestroyInstance(m_graphics_instance, nullptr);
    m_instance = nullptr;
}

void FrameTech::Engine::initialize()
{
    if (m_state == INITIALIZED)
        return;
    if (const auto result = createGraphicsInstance(); result.IsError())
    {
        m_state = ERROR;
        return;
    }
    assert(m_graphics_instance != nullptr);
    if (Result<int> result = createRenderDevice(); result.IsError())
    {
        m_state = ERROR;
        return;
    }
    if (const auto result = pickPhysicalDevice(); result.IsError())
    {
        m_state = ERROR;
        return;
    }
    if (const auto result = m_physical_device.getQueueFamilies(); result.IsError())
    {
        m_state = ERROR;
        return;
    }
    if (const auto result = m_physical_device.createLogicalDevice(); result.IsError())
    {
        m_state = ERROR;
        return;
    }
    assert(m_physical_device.isInitialized());
    m_state = INITIALIZED;
}

FrameTech::Engine* FrameTech::Engine::getInstance()
{
    if (m_instance == nullptr)
    {
        Log("> Instanciating a new Engine singleton");
        m_instance = new FrameTech::Engine();
    }
    return m_instance;
}

FrameTech::Engine::State FrameTech::Engine::getState()
{
    return m_state;
}

Result<int> FrameTech::Engine::pickPhysicalDevice()
{
    return m_physical_device.listDevices();
}

Result<int> FrameTech::Engine::createGraphicsInstance()
{
    listSupportedExtensions();
    // Get the supported extensions
    uint32_t extension_count = 0;
    const char** extension_names = nullptr;
    getRequiredExtensions(&extension_names, &extension_count);

    VkApplicationInfo application_info = createApplicationInfo();

    // Global extensions and validation layers
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extension_names;
    if (VALIDATION_LAYERS.size() == 0)
    {
        create_info.enabledLayerCount = 0;
    }
    else
    {
        Log("> Enabling %d validation layer(s) for the overall engine:", VALIDATION_LAYERS.size());
        for (int i = 0; i < VALIDATION_LAYERS.size(); i++)
            Log("\t* %s", VALIDATION_LAYERS[i]);
        create_info.enabledLayerCount = VALIDATION_LAYERS.size();
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    VkResult instance_creation_result;
    instance_creation_result = vkCreateInstance(&create_info, nullptr, &m_graphics_instance);
    if (instance_creation_result != VK_SUCCESS)
    {
        char* error_msg;
        switch (instance_creation_result)
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
            case VK_ERROR_LAYER_NOT_PRESENT:
                error_msg = (char*)"layer is not present";
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                error_msg = (char*)"extension not present";
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                error_msg = (char*)"incompatible driver";
                break;
            default:
                Log("> vkCreateInstance: error 0x%08x", instance_creation_result);
                error_msg = (char*)"undocumented error";
        }
        LogE("> vkCreateInstance: %s", error_msg);
        return Result<int>::Error(error_msg);
    }
    Log("> The graphics instance has been successfully created");
    return Result<int>::Ok(RESULT_OK);
}

Result<int> FrameTech::Engine::createRenderDevice()
{
    Log("> Creating the render device...");
    if (m_render == nullptr)
        m_render = std::unique_ptr<FrameTech::Graphics::Render>(FrameTech::Graphics::Render::getInstance());
    Result<int> result = m_render->createSurface();
    return result;
}
