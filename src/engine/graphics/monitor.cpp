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
        m_current_monitor = monitors.first[0];
    }
    else
    {
        const std::optional<int> monitor_index = checkForCurrentMonitor(app_window, monitors.first, monitors.second);
        if (monitor_index.has_value())
            m_current_monitor = monitors.first[monitor_index.value()];
    }
    if (nullptr == m_current_monitor)
        return ftstd::VResult::Error((char*)"checkForCurrentMonitor: error getting the primary monitor of the physical device");
    const auto query_monitor_properties = queryProperties(m_current_monitor);
    if (query_monitor_properties.IsError())
    {
        return ftstd::VResult::Error((char*)"queryProperties: error querying properties for the current monitor");
    }
    m_current_monitor_properties = query_monitor_properties.GetValue();
    Log("> Principal monitor properties:");
    Log("\t> tag: '%s'", m_current_monitor_properties.m_name);
    Log("\t> height: %d pixels, width: %d pixels", m_current_monitor_properties.m_height, m_current_monitor_properties.m_width);
    if (nullptr != m_current_monitor_properties.m_current_video_mode)
        Log("\t> refresh rate: %d Hz", m_current_monitor_properties.m_current_video_mode->refreshRate);
    return ftstd::VResult::Ok();
}

frametech::graphics::MonitorProperties& frametech::graphics::Monitor::getCurrentProperties()
{
    return m_current_monitor_properties;
}

bool frametech::graphics::Monitor::foundCurrentMonitor() const
{
    return nullptr != m_current_monitor;
}

ftstd::Result<frametech::graphics::MonitorProperties> frametech::graphics::Monitor::queryProperties(GLFWmonitor* monitor)
{
    if (nullptr == monitor)
        return ftstd::Result<frametech::graphics::MonitorProperties>::Error((char*)"No monitor to query");
    int nb_video_modes{};
    auto monitor_properties = frametech::graphics::MonitorProperties{};
    monitor_properties.m_available_video_modes = glfwGetVideoModes(monitor, &nb_video_modes);
    monitor_properties.m_current_video_mode = glfwGetVideoMode(monitor);
    monitor_properties.m_width = monitor_properties.m_current_video_mode->width;
    monitor_properties.m_height = monitor_properties.m_current_video_mode->height;
    const auto monitor_name = glfwGetMonitorName(monitor);
    if (nullptr != monitor_name) strncpy(monitor_properties.m_name, monitor_name, frametech::graphics::MONITOR_NAME_LEN);
    return ftstd::Result<frametech::graphics::MonitorProperties>::Ok(monitor_properties);
}
