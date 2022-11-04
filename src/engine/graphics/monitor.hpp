//
//  monitor.hpp
//  FrameTech
//
//  Created by Antonin on 04/11/2022.
//

#pragma once
#ifndef monitor_h
#define monitor_h

#include "../../ftstd/result.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <utility>

namespace frametech
{
    namespace graphics
    {
        constexpr size_t MONITOR_NAME_LEN = 32;
        /// @brief Stores the properties of the current / used
        /// GLFWmonitor object
        struct MonitorProperties
        {
            int m_width = 0;  // in pixels
            int m_height = 0; // in pixels
            char m_name[MONITOR_NAME_LEN];
            const GLFWvidmode* m_available_video_modes = nullptr;
            const GLFWvidmode* m_current_video_mode = nullptr;
        };
        /// @brief Wraps a GLFWmonitor object
        class Monitor
        {
        private:
            /// @brief The number of available monitors
            // uint32_t m_available_monitors_list_count = 0;
            /// @brief The primary monitor object
            GLFWmonitor* m_primary_monitor = nullptr;
            /// @brief Stores all properties of the primary monitor
            MonitorProperties m_primary_monitor_properties{};
            /// @brief Internal call to query all properties about the primary monitor
            /// @return A VResult type
            ftstd::VResult queryProperties();

        public:
            Monitor();
            ~Monitor();
            /// @brief Retrieves the list of monitors
            /// @return A pair of information:
            /// 1. The list of GLFWmonitor objects,
            /// 2. The number of retrieved GLFWmonitor objects
            std::pair<GLFWmonitor**, int> scanForMonitors();
            /// @brief Scans for a primary monitor
            /// @return A VResult type
            ftstd::VResult scanForPrimaryMonitor();
            /// @brief Allows the user to select the primary monitor he wants
            /// @param index An index to use in the list of available monitors
            /// @return A VResult type
            ftstd::VResult choosePrimaryMonitor(uint32_t index);
            /// @brief Returns the properties of the primary monitor
            /// @return The properties of the primary monitor
            MonitorProperties& getCurrentProperties();
            /// @brief Returns if the primary monitor has been found (or not)
            /// @return A boolean value
            bool foundPrimaryMonitor() const;
        };
    } // namespace graphics
} // namespace frametech

#endif /* monitor_h */
