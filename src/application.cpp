//
//  application.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#include "application.hpp"
#include "engine/graphics/transform.hpp" // Should be elsewhere
#include "ftstd/debug_tools.h"
#include "ftstd/profile_tools.h"
#include "project.hpp"

#include <filesystem>
#include <stdio.h>

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
    // Force override in order to destroy the internal state of the World object
    m_world.clean();
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#ifndef __APPLE__ // Not supported on Apple platforms for now
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
    (void)io;
    Log(">> ImGui context has been correctly created");
    ImGui::StyleColorsDark();
    Log(">> Setting up the Vulkan renderer...");

    ImGui_ImplGlfw_InitForVulkan(m_app_window, true);
    ImGui_ImplVulkan_InitInfo init_info{};
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
        // TODO: get the internal frame counter with recorded_frames variable, instead of ImGui
        ImGui::Text("Running average %.3f ms/frame (%.1f FPS) (%llu drawed frames)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, m_current_frame);
        if (ImGui::TreeNode("Properties"))
        {
            ImGui::Text("Window size: %dx%d", m_app_width, m_app_height);
            {
                const auto swapchain_extent = m_engine->getInstance()->m_swapchain->getExtent();
                ImGui::Text("Viewport size: %dx%d", swapchain_extent.width, swapchain_extent.height);
            }
            if (GAME_APPLICATION_SETTINGS->fps_target.has_value())
                ImGui::Text("App is limited to %d FPS", GAME_APPLICATION_SETTINGS->fps_target.value());
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
            if (stats.allocationBytes > 1024)
            {
                ImGui::Text("VmaAllocation objects memory: %llukB", stats.allocationBytes >> 10);
            }
            else
            {
                ImGui::Text("VmaAllocation objects memory: %lluB", stats.allocationBytes);
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
#ifdef PROFILE
        if (ImGui::TreeNode("Timers"))
        {
            std::map<std::string, double>::iterator it;
            for (it = ftstd::profile::s_PROFILE_MARKERS.begin(); it != ftstd::profile::s_PROFILE_MARKERS.end(); ++it) {
                ImGui::Text("%s => %f ms", it->first.c_str(), it->second);
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
#endif
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

    if (ImGui::CollapsingHeader("World"))
    {
        frametech::gameframework::World& current_world = frametech::Application::getInstance("")->getCurrentWorld();
        frametech::gameframework::Camera& main_camera = current_world.getMainCamera();
        ImGui::Text("Selected object: (%p)", current_world.getSelectedObject());
        if (ImGui::TreeNode("Camera"))
        {
            ImGui::Text("FOV: %f", main_camera.getFOV());
            ImGui::Text("Type: %s", main_camera.getTypeName().c_str());
            {
                const auto camera_direction = main_camera.getTarget();
                ImGui::Text("Direction: %f,%f,%f", camera_direction.x, camera_direction.y, camera_direction.z);
                if (ImGui::Button("Reset direction"))
                {
                    main_camera.resetTarget();
                }
            }
            {
                const auto camera_position = main_camera.getPosition();
                ImGui::Text("Position: %f,%f,%f", camera_position.x, camera_position.y, camera_position.z);
                if (ImGui::Button("Reset position"))
                {
                    main_camera.resetPosition();
                }
            }
            ImGui::TreePop();
            ImGui::Separator();
        }
        // Only one object (mesh) for the moment, so should be ok
        // But this could be way better if the World object handles itself the objects / meshes
        const frametech::graphics::Mesh c_mesh = m_engine->m_render->getGraphicsPipeline()->getMesh();
        if (ImGui::TreeNode(c_mesh.m_name))
        {
            ImGui::Text("Name: '%s'", c_mesh.m_name);
            ImGui::Text("%lu vertices", c_mesh.m_vertices.size());
            if (ImGui::TreeNode("Vertices"))
            {
                const auto vertices = m_engine->m_render->getGraphicsPipeline()->getVertices();
                size_t i = 0;
                for (const auto vertex : vertices)
                {
                    char vertex_str[80];
                    frametech::engine::graphics::shaders::VertexUtils::toString(vertex_str, vertex);
                    ImGui::Text("%zu:", i);
                    ImGui::SameLine(50);
                    ImGui::Text("%s", vertex_str);
                    ++i;
                }
                ImGui::TreePop();
                ImGui::Separator();
            }
            ImGui::Text("%lu indices", c_mesh.m_indices.size());
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
        if (ImGui::BeginListBox("Meshes"))
        {
            const char* items[] = {"Basic triangle", "Basic quad", "Two quads", "None", "Viking room"};
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
                    m_engine->m_render->getGraphicsPipeline()->setMesh2D(frametech::graphics::Mesh2D(item_current_idx));
                }
            }
            ImGui::EndListBox();
        }
    }

    if (ImGui::CollapsingHeader("Available transforms"))
    {
        if (ImGui::BeginListBox("Transformations"))
        {
            const char* items[] = {"Constant", "Rotate", "Rotate and scale"};
            static int item_current_idx = m_engine->m_render->getGraphicsPipeline()->getTransform();
            int previously_selected_idx = m_engine->m_render->getGraphicsPipeline()->getTransform();
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
                            m_engine->m_render->getGraphicsPipeline()->setTransform(frametech::graphics::Transformation::Constant);
                            break;
                        case 1:
                            m_engine->m_render->getGraphicsPipeline()->setTransform(frametech::graphics::Transformation::Rotate);
                            break;
                        case 2:
                            m_engine->m_render->getGraphicsPipeline()->setTransform(frametech::graphics::Transformation::RotateAndScale);
                            break;
                        default:
                            LogW("item %d cannot be selected in transformation selection view");
                            break;
                    }
                }
            }
            ImGui::EndListBox();
        }
    }

    if (ImGui::CollapsingHeader("Available textures"))
    {
        if (ImGui::BeginListBox("Textures"))
        {
            std::vector<std::string> items = std::vector<std::string>(m_world.m_textures_cache.size());
            int index = 0;
            for (const auto& [texture_id, _] : m_world.m_textures_cache)
            {
                items[index++] = texture_id;
            }
            static std::string item_current_selected = m_world.getSelectedTexture();
            std::string previously_selected_item = m_world.getSelectedTexture();
            for (int n = 0; n < items.size(); ++n)
            {
                const bool is_selected = (items[n] == previously_selected_item);
                if (ImGui::Selectable(items[n].c_str(), is_selected))
                    item_current_selected = items[n];
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected && (item_current_selected != previously_selected_item))
                {
                    previously_selected_item = item_current_selected;
                    ImGui::SetItemDefaultFocus();
                    m_world.setSelectedTexture(item_current_selected);
                    m_engine->m_render->getGraphicsPipeline()->updateDescriptorSets();
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

bool frametech::Application::initEngine()
{
    m_engine = std::unique_ptr<frametech::Engine>(frametech::Engine::getInstance());
    m_engine->initialize();
    return frametech::Engine::State::INITIALIZED == m_engine->getState();
}

void frametech::Application::initDescriptorSets()
{
    assert(nullptr != m_engine);
    m_engine->m_render->getGraphicsPipeline()->createDescriptorSets();
}

ftstd::VResult frametech::Application::loadGameAssets() noexcept
{
    // The game world **should not** be loaded
    assert(!m_world.hasBeenSetup());
    if (GAME_APPLICATION_SETTINGS->asset_folders.empty())
    {
        LogW("No assets have been set for this application");
        return ftstd::VResult::Ok();
    }
    for (const std::string& asset_lib_name : GAME_APPLICATION_SETTINGS->asset_folders)
    {
        if (!std::filesystem::exists(asset_lib_name))
        {
            LogE("Error checking for asset folder '%s' : does not exists", asset_lib_name.c_str());
            continue;
        }

        if (!std::filesystem::is_directory(asset_lib_name)) {
            LogE("Error checking for asset folder '%s' : is not a directory", asset_lib_name.c_str());
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator(asset_lib_name)) {
            const std::string entry_string = entry.path().string();
            const std::string entry_filename = entry.path().filename().string();
            const char* entry_name = entry_string.c_str();
            const auto entry_size = entry.file_size();
            Log("Checking for asset file with name '%s'", entry_name);

            char* contents = new char[entry_size];

            FILE* pFile = fopen(entry_name, "rb");
            if (NULL == pFile) {
                fclose(pFile);
                delete[] contents;
                LogE("Error opening file with path '%s'", entry_name);
            }
            fread(contents, 1, entry_size, pFile);

            m_world.m_textures_cache[entry_filename.c_str()] = new frametech::engine::graphics::Texture();
            if (m_world.m_textures_cache[entry_filename.c_str()]->setup(contents,
                (int)entry_size,
                true,
                frametech::engine::graphics::Texture::Type::T2D,
                VK_FORMAT_R8G8B8A8_SRGB,
                entry_filename.c_str())
                .IsError())
            {
                m_world.m_textures_cache.erase(entry_filename.c_str());
            }
            else
            {
                m_world.setSelectedTexture(entry_filename.c_str());
            }

            fclose(pFile);
            delete[] contents;
        }
    }
    return ftstd::VResult::Error((char*)"Unimplemented loadGameAssets function");
}

void frametech::Application::forceRendererFPSLimit(uint8_t new_limit)
{
    if (GAME_APPLICATION_SETTINGS->fps_target == std::nullopt)
        Log("Setting FPS limit to %d", new_limit);
    else if (new_limit > 0)
        Log("Replacing FPS limit from %d to %d", GAME_APPLICATION_SETTINGS->fps_target, new_limit);
    else
        Log("Disabling FPS limit");
    GAME_APPLICATION_SETTINGS->fps_target = new_limit > 0 ? std::optional<uint8_t>(new_limit) : std::nullopt;
}

void frametech::Application::drawFrame()
{
    ftstd::profile::ScopedProfileMarker scope((char*)"frametech::Application::drawFrame");
    // Update the UBOs
    const uint32_t current_frame_index = m_engine->m_render->getFrameIndex();
    {
        const VkExtent2D& swapchain_extent = m_engine->m_swapchain->getExtent();
        static std::chrono::steady_clock::time_point start_time = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::time_point current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        ModelViewProjection mvp = frametech::graphics::computeTransform(
            m_engine->m_render->getGraphicsPipeline()->getTransform(),
            delta_time,
            swapchain_extent.height,
            swapchain_extent.width);

        m_engine->m_render->getGraphicsPipeline()->updateUniformBuffer(current_frame_index, mvp);
    }

    if (GAME_APPLICATION_SETTINGS->fps_target != std::nullopt)
    {
        const double wait_ms = 1000.0f / GAME_APPLICATION_SETTINGS->fps_target.value();
        double wait_until_ms = ftstd::Timer::get_time_limit(wait_ms);
        // Log("Drawing frame %d...", m_current_frame);

        // Real rendering time
        auto begin_real_rendering_timer = ftstd::Timer();
        m_engine->m_render->getGraphicsPipeline()->acquireImage();
        m_engine->m_render->getGraphicsPipeline()->draw();
        {
            ftstd::profile::ScopedProfileMarker scope((char*)"graphics::present");
            m_engine->m_render->getGraphicsPipeline()->present();
        }
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

#ifdef PROFILE
    // Reset the profile data
    ftstd::profile::s_PROFILE_MARKERS.clear();
#endif
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    frametech::gameframework::World& current_world = frametech::Application::getInstance("")->getCurrentWorld();
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
    {
        current_world.getMainCamera().setType(frametech::gameframework::Camera::Type::STATIONARY);
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::ALT_UP_COMBINED);
        if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::ALT_DOWN_COMBINED);
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::ALT_LEFT_COMBINED);
        if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::ALT_RIGHT_COMBINED);
    }
    else
    {
        current_world.getMainCamera().setType(frametech::gameframework::Camera::Type::WORLD);
        if (key == GLFW_KEY_UP || key == GLFW_KEY_W)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::UP);
        if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::DOWN);
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::LEFT);
        if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D)
            frametech::Application::getInstance("")->m_key_events_handler.addKey(frametech::engine::inputs::Key::RIGHT);
    }
}

static void cursorCallback(GLFWwindow* window, const double xpos, const double ypos) {
    frametech::Application::getInstance("")->m_cursor_events_handler.addMove((float)xpos, (float)ypos);
    // TODO : update camera direction
#ifdef IMGUI
    // As ImGui is now using cursor callbacks too, we need to forward
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
#endif
}

void frametech::Application::run()
{
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
#ifdef IMGUI
            setupImGui();
            uploadImGuiFont();
#endif
            Log("> Application loop...");
            if (GAME_APPLICATION_SETTINGS->fps_target.has_value() && nullptr != m_monitor.getCurrentProperties().m_current_video_mode)
            {
                // Set the refresh rate of the monitor by default
                const auto monitor_refresh_rate = m_monitor.getCurrentProperties().m_current_video_mode->refreshRate;
                const auto c_FPS_limit = GAME_APPLICATION_SETTINGS->fps_target.value();
                // Cap to the supported refrest rate
                // As an example: no 120FPS is the monitor is capped to 60Hz
                if (c_FPS_limit > monitor_refresh_rate)
                {
                    LogW("The monitor is using a refresh rate lower than the current cap setting (set to %d) - engine lowered it to %d FPS", c_FPS_limit, monitor_refresh_rate);
                    GAME_APPLICATION_SETTINGS->fps_target = monitor_refresh_rate;
                }
            }
#if defined(DEBUG) || defined(PROFILE)
            GAME_APPLICATION_SETTINGS->fps_target.has_value() ? Log("> Application is running at %d FPS", GAME_APPLICATION_SETTINGS->fps_target.value()) : Log("> Application is running at unlimited frame");
#endif
            // Initialize our world
            m_world.setup();
            // Initialize the key events
            glfwSetKeyCallback(m_app_window, keyCallback);
            // Initialize the mouse events
            glfwSetCursorPosCallback(m_app_window, cursorCallback);
            m_state = frametech::Application::State::RUNNING;
            while (!glfwWindowShouldClose(m_app_window) && m_state == frametech::Application::State::RUNNING)
            {
                glfwPollEvents();
                m_key_events_handler.poll(false);
                m_cursor_events_handler.poll(false);
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
#ifdef IMGUI
            cleanImGui();
#endif
        }
        break;
    }
    m_state = frametech::Application::State::CLOSING;
}

uint64_t frametech::Application::getCurrentFrame()
{
    return m_current_frame;
}

frametech::gameframework::World& frametech::Application::getCurrentWorld() noexcept
{
    return m_world;
}
