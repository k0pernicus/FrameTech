//
//  engine.cpp
//  FrameTech
//
//  Created by Antonin on 24/09/2022.
//

#include "engine.hpp"
#include "../application.hpp"
#include "../ftstd/debug_tools.h"
#include "../project.hpp"

#ifdef DEBUG
const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> REQUIRED_EXTENSIONS = {
    "VK_EXT_debug_utils",
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
};
#else
const std::vector<const char*> VALIDATION_LAYERS = {};
const std::vector<const char*> REQUIRED_EXTENSIONS = {
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
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

frametech::Engine* frametech::Engine::m_instance{nullptr};

frametech::Engine::Engine()
{
    m_state = State::UNINITIALIZED;
}

frametech::Engine::~Engine()
{
    Log("< Closing the Engine object...");
    m_swapchain = nullptr;
    m_render = nullptr;
    if (m_descriptor_pool)
        vkDestroyDescriptorPool(m_graphics_device.getLogicalDevice(), m_descriptor_pool, nullptr);
    if (VK_NULL_HANDLE != m_allocator)
        vmaDestroyAllocator(m_allocator);
    m_graphics_device.Destroy();
    if (m_graphics_instance)
        vkDestroyInstance(m_graphics_instance, nullptr);
    m_instance = nullptr;
}

void frametech::Engine::initialize()
{
    if (m_state == State::INITIALIZED)
        return;
    if (const auto result = createGraphicsInstance(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    assert(m_graphics_instance != nullptr);
    if (const auto result = createRenderDevice(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = pickPhysicalDevice(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = m_graphics_device.getQueueFamilies(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = m_graphics_device.createLogicalDevice(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = createAllocator(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = createDescriptorPool(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = createSwapChain(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = m_render->createImageViews(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = m_render->createGraphicsPipeline(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    if (const auto result = m_render->createFramebuffers(); result.IsError())
    {
        m_state = State::ERROR;
        return;
    }
    assert(m_graphics_device.isInitialized());
    m_state = State::INITIALIZED;
}

frametech::Engine* frametech::Engine::getInstance()
{
    if (nullptr == m_instance)
    {
        Log("> Instanciating a new Engine singleton");
        m_instance = new frametech::Engine();
    }
    return m_instance;
}

frametech::Engine::State frametech::Engine::getState()
{
    return m_state;
}

ftstd::VResult frametech::Engine::pickPhysicalDevice()
{
    return m_graphics_device.listDevices();
}

ftstd::VResult frametech::Engine::createAllocator()
{
    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.vulkanApiVersion = VK_MAKE_VERSION(
        Project::VULKAN_MIN_VERSION_MAJOR,
        Project::VULKAN_MIN_VERSION_MINOR,
        Project::VULKAN_MIN_VERSION_BUGFIX);
    allocator_create_info.physicalDevice = m_graphics_device.getPhysicalDevice();
    allocator_create_info.device = m_graphics_device.getLogicalDevice();
    allocator_create_info.instance = m_graphics_instance;
    if (const auto result = vmaCreateAllocator(&allocator_create_info, &m_allocator); result == VK_SUCCESS)
        return ftstd::VResult::Ok();
    return ftstd::VResult::Error((char*)"Failed to initialize the internal allocator");
}

ftstd::VResult frametech::Engine::createGraphicsInstance()
{
    listSupportedExtensions();
    // Get the supported extensions
    uint32_t extension_count = 0;
    // Platform specific additions
    const char** extension_names = glfwGetRequiredInstanceExtensions(&extension_count);

    VkApplicationInfo application_info = createApplicationInfo();

    // Global extensions and validation layers
    VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        // For macOS >= 1.3.216
        // https://vulkan.lunarg.com/doc/sdk/1.3.216.0/mac/getting_started.html
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extension_names,
    };
    if (VALIDATION_LAYERS.size() == 0)
    {
        create_info.enabledLayerCount = 0;
    }
    else
    {
        Log("> Enabling %d validation layer(s) for the overall engine:", VALIDATION_LAYERS.size());
        for (int i = 0; i < VALIDATION_LAYERS.size(); i++)
            Log("\t* %s", VALIDATION_LAYERS[i]);
        create_info.enabledLayerCount = (uint32_t)VALIDATION_LAYERS.size();
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
                LogE("> vkCreateInstance: error 0x%08x", instance_creation_result);
                error_msg = (char*)"undocumented error";
        }
        LogE("> vkCreateInstance: %s", error_msg);
        return ftstd::VResult::Error(error_msg);
    }
    Log("> The graphics instance has been successfully created");

    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::Engine::createRenderDevice()
{
    Log("> Creating the render device...");
    if (nullptr == m_render)
        m_render = std::unique_ptr<frametech::graphics::Render>(frametech::graphics::Render::getInstance());
    ftstd::VResult result = m_render->createSurface();
    return result;
}

ftstd::VResult frametech::Engine::createSwapChain()
{
    Log("> Creating the swapchain...");
    if (nullptr == m_swapchain)
        m_swapchain = std::unique_ptr<frametech::graphics::SwapChain>(frametech::graphics::SwapChain::getInstance());
    m_swapchain->queryDetails();
    return m_swapchain->create();
}

ftstd::VResult frametech::Engine::createDescriptorPool()
{
    Log("> Creating the descriptor pool...");
    VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    const auto pool_sizes_count = (int)(sizeof(pool_sizes) / sizeof(*(pool_sizes)));
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * pool_sizes_count;
    pool_info.poolSizeCount = (uint32_t)pool_sizes_count;
    pool_info.pPoolSizes = pool_sizes;
    if (const auto result_status = vkCreateDescriptorPool(m_graphics_device.getLogicalDevice(), &pool_info, nullptr, &m_descriptor_pool); result_status != VK_SUCCESS)
    {
        LogE("> vkCreateDescriptorPool: cannot create the descriptor pool");
        return ftstd::VResult::Error((char*)"Cannot create the descriptor pool");
    }
    return ftstd::VResult::Ok();
}

VkDescriptorPool frametech::Engine::getDescriptorPool() const noexcept
{
    return m_descriptor_pool;
}
