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

static void getSupportedExtensions(const char*** extension_names, uint32_t* extension_count)
{
    (*extension_names) = glfwGetRequiredInstanceExtensions(extension_count);
    if ((*extension_count) == 0)
    {
        // Is this really an error?
        LogE("> no supported extension found...");
        return;
    }
    for (int i = 0; i < (*extension_count); i++)
        Log("> supported extension %d -> %s", i, (*extension_names)[i]);
}

FrameTech::Engine* FrameTech::Engine::m_instance{nullptr};

FrameTech::Engine::Engine()
{
    m_state = State::UNINITIALIZED;
}

FrameTech::Engine::~Engine()
{
    Log("< Closing the Engine object...");
    m_instance = nullptr;
}

void FrameTech::Engine::initialize()
{
    if (m_state == INITIALIZED)
        return;
    createGraphicsInstance();
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

void FrameTech::Engine::createGraphicsInstance()
{
    // Get the supported extensions
    uint32_t extension_count = 0;
    const char** extension_names = nullptr;
    getSupportedExtensions(&extension_names, &extension_count);

    VkApplicationInfo application_info = createApplicationInfo();

    // Global extensions and validation layers
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extension_names;
    create_info.enabledLayerCount = 0; // TODO: change

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
        return;
    }
    Log("> the graphics instance has been successfully created");
}
