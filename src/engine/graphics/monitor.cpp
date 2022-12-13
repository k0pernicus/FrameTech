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

std::pair<GLFWmonitor**, int> frametech::graphics::Monitor::scanForMonitors()
{
    int nb_monitors{};
    GLFWmonitor** monitors = glfwGetMonitors(&nb_monitors);
    return std::pair<GLFWmonitor**, int>({monitors, nb_monitors});
}

ftstd::VResult frametech::graphics::Monitor::scanForPrimaryMonitor()
{
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary == nullptr)
        return ftstd::VResult::Error((char*)"Error getting the primary monitor of the physical device");
    m_primary_monitor = primary;
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
