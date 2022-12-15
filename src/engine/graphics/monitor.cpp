//
//  monitor.cpp
//  FrameTech
//
//  Created by Antonin on 04/11/2022.
//

#include "monitor.hpp"
#include "../../ftstd/debug_tools.h"
#include <GLFW/glfw3.h>
#include <string.h>

frametech::graphics::Monitor::Monitor() {}

frametech::graphics::Monitor::~Monitor() {}

std::optional<int> frametech::graphics::Monitor::checkForCurrentMonitor(GLFWwindow* window, GLFWmonitor** monitors, const int monitors_size) noexcept
{
    int window_rectangle[4] = {0};
    glfwGetWindowPos(window, &window_rectangle[0], &window_rectangle[1]);
    glfwGetWindowSize(window, &window_rectangle[2], &window_rectangle[3]);

    std::optional<int> closest_monitor = std::nullopt;
    int max_overlap_area = 0;

    for (int i = 0; i < monitors_size; ++i)
    {
        int monitor_rectangle[4] = {0};
        glfwGetMonitorPos(monitors[i], &monitor_rectangle[0], &monitor_rectangle[1]);
        const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

        monitor_rectangle[2] = monitor_video_mode->width;
        monitor_rectangle[3] = monitor_video_mode->height;

        if (
            !(
                ((window_rectangle[0] + window_rectangle[2]) < monitor_rectangle[0]) ||
                (window_rectangle[0] > (monitor_rectangle[0] + monitor_rectangle[2])) ||
                ((window_rectangle[1] + window_rectangle[3]) < monitor_rectangle[1]) ||
                (window_rectangle[1] > (monitor_rectangle[1] + monitor_rectangle[3]))))
        {
            int intersection_rectangle[4] = {0};

            // x, width
            if (window_rectangle[0] < monitor_rectangle[0])
            {
                intersection_rectangle[0] = monitor_rectangle[0];

                if ((window_rectangle[0] + window_rectangle[2]) < (monitor_rectangle[0] + monitor_rectangle[2]))
                {
                    intersection_rectangle[2] = (window_rectangle[0] + window_rectangle[2]) - intersection_rectangle[0];
                }
                else
                {
                    intersection_rectangle[2] = monitor_rectangle[2];
                }
            }
            else
            {
                intersection_rectangle[0] = window_rectangle[0];

                if ((monitor_rectangle[0] + monitor_rectangle[2]) < (window_rectangle[0] + window_rectangle[2]))
                {
                    intersection_rectangle[2] = (monitor_rectangle[0] + monitor_rectangle[2]) - intersection_rectangle[0];
                }
                else
                {
                    intersection_rectangle[2] = window_rectangle[2];
                }
            }

            // y, height
            if (window_rectangle[1] < monitor_rectangle[1])
            {
                intersection_rectangle[1] = monitor_rectangle[1];

                if ((window_rectangle[1] + window_rectangle[3]) < (monitor_rectangle[1] + monitor_rectangle[3]))
                {
                    intersection_rectangle[3] = (window_rectangle[1] + window_rectangle[3]) - intersection_rectangle[1];
                }
                else
                {
                    intersection_rectangle[3] = monitor_rectangle[3];
                }
            }
            else
            {
                intersection_rectangle[1] = window_rectangle[1];

                if ((monitor_rectangle[1] + monitor_rectangle[3]) < (window_rectangle[1] + window_rectangle[3]))
                {
                    intersection_rectangle[3] = (monitor_rectangle[1] + monitor_rectangle[3]) - intersection_rectangle[1];
                }
                else
                {
                    intersection_rectangle[3] = window_rectangle[3];
                }
            }

            int overlap_area = intersection_rectangle[2] * intersection_rectangle[3];
            if (overlap_area > max_overlap_area)
            {
                closest_monitor = i;
                max_overlap_area = overlap_area;
            }
        }
    }

    return closest_monitor;
}

std::pair<GLFWmonitor**, int> frametech::graphics::Monitor::scanForMonitors() noexcept
{
    int nb_monitors{};
    GLFWmonitor** monitors = glfwGetMonitors(&nb_monitors);
    return std::pair<GLFWmonitor**, int>({monitors, nb_monitors});
}

ftstd::VResult frametech::graphics::Monitor::scanForCurrentMonitor(GLFWwindow* app_window) noexcept
{
    const std::pair<GLFWmonitor**, int> monitors = scanForMonitors();
    if (monitors.second == 0)
        return ftstd::VResult::Error((char*)"Error getting the primary monitor of the physical device");
    if (monitors.second == 1)
    {
        m_primary_monitor = monitors.first[0];
    }
    else
    {
        const std::optional<int> monitor_index = checkForCurrentMonitor(app_window, monitors.first, monitors.second);
        if (monitor_index.has_value())
            m_primary_monitor = monitors.first[monitor_index.value()];
    }
    if (nullptr == m_primary_monitor)
        return ftstd::VResult::Error((char*)"Error getting the primary monitor of the physical device");
    if (const auto query_result_code = queryProperties(); query_result_code.IsError())
    {
        LogE("> Error querying information about the primary monitor");
    }
    Log("> Principal monitor properties:");
    Log("\t> tag: '%s'", m_primary_monitor_properties.m_name);
    Log("\t> height: %d pixels, width: %d pixels", m_primary_monitor_properties.m_height, m_primary_monitor_properties.m_width);
    if (nullptr != m_primary_monitor_properties.m_current_video_mode)
        Log("\t> refresh rate: %d Hz", m_primary_monitor_properties.m_current_video_mode->refreshRate);
    return ftstd::VResult::Ok();
}

ftstd::VResult frametech::graphics::Monitor::choosePrimaryMonitor(uint32_t index)
{
    // TODO with callbacks
    WARN_RT_UNIMPLEMENTED;
    return ftstd::VResult::Ok();
}

frametech::graphics::MonitorProperties& frametech::graphics::Monitor::getCurrentProperties()
{
    return m_primary_monitor_properties;
}

bool frametech::graphics::Monitor::foundPrimaryMonitor() const
{
    return m_primary_monitor != nullptr;
}

ftstd::VResult frametech::graphics::Monitor::queryProperties()
{
    if (m_primary_monitor == nullptr)
        return ftstd::VResult::Error((char*)"No primary monitor to query");
    int nb_video_modes{};
    m_primary_monitor_properties = frametech::graphics::MonitorProperties{};
    m_primary_monitor_properties.m_available_video_modes = glfwGetVideoModes(m_primary_monitor, &nb_video_modes);
    m_primary_monitor_properties.m_current_video_mode = glfwGetVideoMode(m_primary_monitor);
    m_primary_monitor_properties.m_width = m_primary_monitor_properties.m_current_video_mode->width;
    m_primary_monitor_properties.m_height = m_primary_monitor_properties.m_current_video_mode->height;
    const auto monitor_name = glfwGetMonitorName(m_primary_monitor);
    if (nullptr != monitor_name)
        strncpy(m_primary_monitor_properties.m_name, monitor_name, frametech::graphics::MONITOR_NAME_LEN);
    return ftstd::VResult::Ok();
}
