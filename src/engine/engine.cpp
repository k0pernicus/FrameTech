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
    (*extension_names) = glfwGetRequiredInstanceExtensions(extension_count);
    if ((*extension_count) == 0)
    {
        // Is this really an error?
        LogE("> No required extension found...");
        return;
    }
    Log("> Required extensions:");
    for (int i = 0; i < (*extension_count); i++)
        Log("\t* %d -> %s", i, (*extension_names)[i]);
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
    Log("> Supported extensions:");
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
    if (m_graphics_instance)
        vkDestroyInstance(m_graphics_instance, nullptr);
    m_instance = nullptr;
}

void FrameTech::Engine::initialize()
{
    if (m_state == INITIALIZED)
        return;
    const auto result = createGraphicsInstance();
    m_state = result.IsError() ? ERROR : INITIALIZED;
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
    create_info.enabledLayerCount = 0; // TODO: change

    Result<int> result{};
    VkResult instance_creation_result;
    instance_creation_result = vkCreateInstance(&create_info, nullptr, &m_graphics_instance);
    if (instance_creation_result != VK_SUCCESS)
    {
        switch (instance_creation_result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                LogE("> failed to create a graphics instance: out of host memory");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                LogE("> failed to create a graphics instance: out of device memory");
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                LogE("> failed to create a graphics instance: initialization memory");
                break;
            case VK_ERROR_LAYER_NOT_PRESENT:
                LogE("> failed to create a graphics instance: layer is not present");
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                LogE("> failed to create a graphics instance: extension not present");
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                LogE("> failed to create a graphics instance: incompatible driver");
                break;
            default:
                LogE("> failed to create an instance, undocumented error (code 0x%08x)", instance_creation_result);
        }
        result.Error(-1, "failed to create an instance");
        return result;
    }
    Log("> The graphics instance has been successfully created");
    result.Ok(RESULT_OK);
    return result;
}
