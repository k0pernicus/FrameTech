//
//  application.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#include "application.hpp"
#include "ftstd/debug_tools.h"
#include "project.hpp"

// For the transformation / UBOs
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef IMGUI
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

/// @brief Stores if the ImGui demo window should be rendered or not
bool DRAW_IMGUI_APP = false;

extern char S_APP_VERSION[18];
extern char S_ENGINE_VERSION[18];
#endif

frametech::Application* frametech::Application::m_instance{nullptr};

frametech::Application::Application(const char* app_title)
{
    m_app_title = app_title;
    m_app_timer = std::unique_ptr<ftstd::Timer>(new ftstd::Timer());
}

frametech::Application::~Application()
{
    clean();
    Log("< Closing the Application object...");
    m_instance = nullptr;
}

GLFWwindow* frametech::Application::getWindow() const
{
    return m_app_window;
}

frametech::Application* frametech::Application::getInstance(const char* app_title)
{
    if (m_instance == nullptr)
    {
        Log("> Instanciating a new Application singleton");
        m_instance = new frametech::Application(app_title);
    }
    return m_instance;
}

void frametech::Application::clean()
{
    Log("< Cleaning the Application object");
    m_app_title = nullptr;
    m_engine = nullptr;
    glfwDestroyWindow(m_app_window);
    m_app_window = nullptr;
    glfwTerminate();
}

ftstd::VResult frametech::Application::initWindow()
{
    Log("> Initializing the Application window");
    glfwInit();                                   // Initialize the GLFW library
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context, as we use Vulkan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);   // No resizable option for the window
    // Check for Vulkan support
    if (!glfwVulkanSupported())
    {
        return ftstd::VResult::Error((char*)"Vulkan is not supported");
    }
    m_app_height = frametech::DEFAULT_WINDOW_HEIGHT;
    m_app_width = frametech::DEFAULT_WINDOW_WIDTH;
    // The last parameter in glfwCreateWindow is only for OpenGL - no need to setup it here
    m_app_window = glfwCreateWindow(m_app_width,
                                    m_app_height,
                                    m_app_title,
                                    nullptr,
                                    nullptr);
    // Initialize the monitor object
    if (!m_monitor.foundCurrentMonitor())
        if (const auto scan_result_code = m_monitor.scanForCurrentMonitor(m_app_window); scan_result_code.IsError())
            return ftstd::VResult::Error((char*)"Error getting the primary monitor");
    return ftstd::VResult::Ok();
}

#ifdef IMGUI
/// @brief This function should NOT be called **BEFORE** the
/// Vulkan window setup
void frametech::Application::setupImGui()
{
    Log("> Setup ImGui...");
    // Setting up imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Enable docking mode
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    (void)io;
    Log(">> ImGui context has been correctly created");
    ImGui::StyleColorsDark();
    Log(">> Setting up the Vulkan renderer...");

    ImGui_ImplGlfw_InitForVulkan(m_app_window, true);
    ImGui_ImplVulkan_InitInfo init_info{};
    // TODO, setup
    Log("<< Ended up to setup the Vulkan renderer...");
    init_info.Instance = m_engine->m_graphics_instance;
    init_info.PhysicalDevice = m_engine->m_graphics_device.getPhysicalDevice();
    init_info.Device = m_engine->m_graphics_device.getLogicalDevice();
    init_info.Queue = m_engine->m_graphics_device.getGraphicsQueue();
    init_info.QueueFamily = m_engine->m_graphics_device.m_graphics_queue_family_index;
    init_info.DescriptorPool = m_engine->getDescriptorPool();
    // TODO: check to retrieve the information BETTER
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info, m_engine->m_render->getGraphicsPipeline()->getRenderPass());
    Log("<< Ended up the init of ImplVulkan with ImGui...");

    Log("< Ending ImGui setup...");
}

ftstd::VResult frametech::Application::uploadImGuiFont()
{
    Log("> Uploading ImGui font...");
    // Use any command queue
    auto command_buffer_obj = m_engine->m_render->getGraphicsCommand();
    VkCommandPool* command_pool = command_buffer_obj->getPool();
    VkCommandBuffer* command_buffer = command_buffer_obj->getBuffer();
    VkDevice device = m_engine->m_graphics_device.getLogicalDevice();

    if (const auto result_status = vkResetCommandPool(device, *command_pool, 0); result_status != VK_SUCCESS)
    {
        LogE("vkResetCommandPool to upload ImGui font failed");
        return ftstd::VResult::Error((char*)"canno't upload ImGui font");
    }
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (const auto result_status = vkBeginCommandBuffer(*command_buffer, &begin_info); result_status != VK_SUCCESS)
    {
        LogE("vkBeginCommandBuffer to upload ImGui font failed");
        return ftstd::VResult::Error((char*)"canno't upload ImGui font");
    }

    ImGui_ImplVulkan_CreateFontsTexture(*command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = command_buffer;
    if (const auto result_status = vkEndCommandBuffer(*command_buffer); result_status != VK_SUCCESS)
    {
        LogE("vkEndCommandBuffer to upload ImGui font failed");
        return ftstd::VResult::Error((char*)"canno't upload ImGui font");
    }
    if (const auto result_status = vkQueueSubmit(m_engine->m_graphics_device.getGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE); result_status != VK_SUCCESS)
    {
        LogE("vkQueueSubmit to upload ImGui font failed");
        return ftstd::VResult::Error((char*)"canno't upload ImGui font");
    }

    if (const auto result_status = vkDeviceWaitIdle(device); result_status != VK_SUCCESS)
    {
        LogE("vkDeviceWaitIdle to upload ImGui font failed");
        return ftstd::VResult::Error((char*)"canno't upload ImGui font");
    }
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    Log("< Ending up uploading ImGui font...");
    return ftstd::VResult::Ok();
}

void frametech::Application::drawDebugToolImGui()
{
    // Optimization technique
    if (!ImGui::Begin("Debug tool", &DRAW_IMGUI_APP))
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("App"))
    {
        ImGui::Text("Name: '%s'", m_app_title);
        ImGui::SameLine(220);
        ImGui::Text("Version: %s", S_APP_VERSION);
        ImGui::Text("Running average %.3f ms/frame (%.1f FPS) (%llu drawed frames)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, m_current_frame);
        if (ImGui::TreeNode("Properties"))
        {
            ImGui::Text("Size: %dx%d", m_app_width, m_app_height);
            if (m_FPS_limit.has_value())
                ImGui::Text("App is limited to %d FPS", m_FPS_limit.value());
            else
                ImGui::Text("App has no rendering limitation");
            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Engine"))
    {
        ImGui::Text("Name: '%s'", m_engine->getEngineName());
        ImGui::SameLine(220);
        ImGui::Text("Version: %s", S_ENGINE_VERSION);
        if (ImGui::TreeNode("Memory stats"))
        {

            VmaTotalStatistics total_stats;
            VmaAllocator allocator = m_engine->m_allocator;
            vmaCalculateStatistics(allocator, &total_stats);
            const auto stats = total_stats.total.statistics;

            ImGui::Text("Vulkan memory blocks allocated: %d", stats.blockCount);
            ImGui::Text("VmaAllocation objects allocated: %d", stats.allocationCount);
            ImGui::Text("VkDeviceMemory blocks memory: %llu kB", stats.blockBytes >> 10);
            if (stats.allocationBytes > 1024) {
                ImGui::Text("VmaAllocation objects memory: %llukB", stats.allocationBytes >> 10);
            }
            else {
                ImGui::Text("VmaAllocation objects memory: %lluB", stats.allocationBytes);
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Monitor properties"))
    {
        const auto monitor_properties = m_monitor.getCurrentProperties();
        ImGui::Text("Name: %s", monitor_properties.m_name);
        ImGui::Text("Size: %dx%d", monitor_properties.m_width, monitor_properties.m_height);
        ImGui::Text("Refresh rate: %d Hz", monitor_properties.m_current_video_mode->refreshRate);
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("On screen"))
    {
        const frametech::graphics::Mesh c_mesh = m_engine->m_render->getGraphicsPipeline()->getMesh();
        ImGui::Text("Name: '%s'", c_mesh.m_name);
        ImGui::Text("%llu vertices", c_mesh.m_vertices.size());
        if (ImGui::TreeNode("Vertices"))
        {
            const auto vertices = m_engine->m_render->getGraphicsPipeline()->getVertices();
            size_t i = 0;
            for (const auto vertex : vertices)
            {
                char vertex_str[80];
                frametech::shaders::VertexUtils::toString(vertex_str, vertex);
                ImGui::Text("%zu:", i);
                ImGui::SameLine(50);
                ImGui::Text("%s", vertex_str);
                ++i;
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
        ImGui::Text("%llu indices", c_mesh.m_indices.size());
        if (ImGui::TreeNode("Indices"))
        {
            const auto indices = m_engine->m_render->getGraphicsPipeline()->getIndices();
            size_t i = 0;
            for (const auto index : indices)
            {
                ImGui::Text("%zu: vertex %d", i, index);
                ++i;
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    ImGui::Separator();

    ImGui::End();
}

void frametech::Application::drawMeshSelectionImGui()
{

    // Optimization technique
    if (!ImGui::Begin("Mesh selector", &DRAW_IMGUI_APP))
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Available meshes"))
    {
        if (ImGui::BeginListBox("2D meshes"))
        {
            const char* items[] = {"Basic triangle", "Basic quad", "None"};
            static int item_current_idx = m_engine->m_render->getGraphicsPipeline()->getMesh().m_type;
            int previously_selected_idx = m_engine->m_render->getGraphicsPipeline()->getMesh().m_type;
            for (int n = 0; n < sizeof(items) / sizeof(items[0]); ++n)
            {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(items[n], is_selected))
                    item_current_idx = n;
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected && (item_current_idx != previously_selected_idx))
                {
                    previously_selected_idx = item_current_idx;
                    ImGui::SetItemDefaultFocus();
                    switch (item_current_idx)
                    {
                        case 0:
                            m_engine->m_render->getGraphicsPipeline()->setMesh2D(frametech::graphics::Mesh2D::BASIC_TRIANGLE);
                            break;
                        case 1:
                            m_engine->m_render->getGraphicsPipeline()->setMesh2D(frametech::graphics::Mesh2D::BASIC_QUAD);
                            break;
                        case 2:
                            m_engine->m_render->getGraphicsPipeline()->setMesh2D(frametech::graphics::Mesh2D::NONE);
                            break;
                    }
                }
            }
            ImGui::EndListBox();
        }
    }

    ImGui::Separator();

    ImGui::End();
}

void frametech::Application::cleanImGui()
{

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
#endif

void frametech::Application::initEngine()
{
    m_engine = std::unique_ptr<frametech::Engine>(frametech::Engine::getInstance());
    m_engine->initialize();
}

void frametech::Application::forceRendererFPSLimit(uint8_t new_limit)
{
    if (m_FPS_limit == std::nullopt)
        Log("Setting FPS limit to %d", new_limit);
    else if (new_limit > 0)
        Log("Replacing FPS limit from %d to %d", m_FPS_limit, new_limit);
    else
        Log("Disabling FPS limit");
    m_FPS_limit = new_limit > 0 ? std::optional<uint8_t>(new_limit) : std::nullopt;
}

void frametech::Application::drawFrame()
{
    // Update the UBOs
    const uint32_t current_frame_index = m_engine->m_render->getFrameIndex();
    {
        const VkExtent2D& swapchain_extent = m_engine->m_swapchain->getExtent();
        static std::chrono::steady_clock::time_point start_time = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
        UniformBufferObject ubo{
            .model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),                           // 90 degrees
            .view = glm::lookAt(glm::vec3(4.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),                                  // 45 degrees
            .projection = glm::perspective(glm::radians(45.0f) * cos(delta_time), swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f), // 45 degrees
        };
        // ubo.projection[1][1] *= -1; // glm is for OpenGL (Y is inverted)
    
        m_engine->m_render->getGraphicsPipeline()->updateUniformBuffer(current_frame_index, ubo);
    }

    if (m_FPS_limit != std::nullopt)
    {
        const double wait_ms = 1000.0f / m_FPS_limit.value();
        double wait_until_ms = ftstd::Timer::get_time_limit(wait_ms);
        // Log("Drawing frame %d...", m_current_frame);

        // Real rendering time
        auto begin_real_rendering_timer = ftstd::Timer();
        m_engine->m_render->getGraphicsPipeline()->acquireImage();
        m_engine->m_render->getGraphicsPipeline()->draw();
        m_engine->m_render->getGraphicsPipeline()->present();
        const auto rendering_time_diff = begin_real_rendering_timer.diff();

        recorded_frames[recorded_frames_index] = rendering_time_diff > 0 ? rendering_time_diff : 1;
        recorded_frames_index = (recorded_frames_index + 1) % FPS_RECORDS;

        // Force to pause the rendering thread
        // if (and only if) the time has not come yet
        m_app_timer->block_until(static_cast<uint64_t>(wait_until_ms));
        m_engine->m_render->updateFrameIndex(m_current_frame);
        ++m_current_frame;
        return;
    }

    // Real rendering time
    auto begin_real_rendering_timer = ftstd::Timer();
    m_engine->m_render->getGraphicsPipeline()->acquireImage();
    m_engine->m_render->getGraphicsPipeline()->draw();
    m_engine->m_render->getGraphicsPipeline()->present();
    const uint64_t rendering_time_diff = begin_real_rendering_timer.diff();

    recorded_frames[recorded_frames_index] = rendering_time_diff > 0 ? rendering_time_diff : 1;
    recorded_frames_index = (recorded_frames_index + 1) % FPS_RECORDS;

    m_engine->m_render->updateFrameIndex(m_current_frame);
    ++m_current_frame;
}

void frametech::Application::run()
{
#ifdef IMGUI
    setupImGui();
    uploadImGuiFont();
#endif
    switch (m_engine->getState())
    {
        case frametech::Engine::State::UNINITIALIZED:
        {
            LogE("< Running the application if the engine is uninitialized is forbidden!");
        }
        case frametech::Engine::State::ERROR: // Used by UNINITIALIZED as well
        {
            m_state = frametech::Application::State::SHOULD_BE_CLOSED;
        }
        break;
        case frametech::Engine::State::INITIALIZED:
        {
            Log("> Application loop...");
            if (m_FPS_limit.has_value() && nullptr != m_monitor.getCurrentProperties().m_current_video_mode)
            {
                // Set the refresh rate of the monitor by default
                const auto monitor_refresh_rate = m_monitor.getCurrentProperties().m_current_video_mode->refreshRate;
                const auto c_FPS_limit = m_FPS_limit.value();
                // Cap to the supported refrest rate
                // As an example: no 120FPS is the monitor is capped to 60Hz
                if (c_FPS_limit > monitor_refresh_rate)
                {
                    LogW("The monitor is using a refresh rate lower than the current cap setting (set to %d) - engine lowered it to %d FPS", c_FPS_limit, monitor_refresh_rate);
                    m_FPS_limit = monitor_refresh_rate;
                }
            }
#ifdef DEBUG
            m_FPS_limit.has_value() ? Log("> Application is running at %d FPS", m_FPS_limit.value()) : Log("> Application is running at unlimited frame");
#endif
            m_state = frametech::Application::State::RUNNING;
            while (!glfwWindowShouldClose(m_app_window) && m_state == frametech::Application::State::RUNNING)
            {
                glfwPollEvents();
#ifdef IMGUI
                // Log(">> Rendering ImGui");
                // Start the Dear ImGui frame
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                drawDebugToolImGui();
                drawMeshSelectionImGui();
#endif
                // drawFrame includes the acquisition, draw, and present processes
                drawFrame();
            }
            Log("< ...Application loop");
            vkDeviceWaitIdle(m_engine->m_graphics_device.getLogicalDevice());
        }
        break;
    }
#ifdef IMGUI
    cleanImGui();
#endif
    m_state = frametech::Application::State::CLOSING;
}

uint64_t frametech::Application::getCurrentFrame()
{
    return m_current_frame;
}
