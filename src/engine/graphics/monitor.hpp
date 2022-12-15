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
#include <optional>
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
            /// @brief The current monitor object
            GLFWmonitor* m_current_monitor = nullptr;
            /// @brief Stores all properties of the current monitor
            MonitorProperties m_current_monitor_properties{};
            /// @brief Internal call to query all properties about the current monitor
            /// @return A Result type
            static ftstd::Result<frametech::graphics::MonitorProperties> queryProperties(GLFWmonitor* monitor);

        public:
            Monitor();
            ~Monitor();
            /// @brief Returns the index of the monitor that displays the application
            /// @param monitors List of current monitors
            /// @param nb_monitors The numbver of monitors in the previous list
            /// @return An optional type that contains the index of the current monitor
            static std::optional<int> checkForCurrentMonitor(GLFWwindow* window, GLFWmonitor** monitors, const int monitors_size) noexcept;
            /// @brief Retrieves the list of monitors
            /// @return A pair of information:
            /// 1. The list of GLFWmonitor objects,
            /// 2. The number of retrieved GLFWmonitor objects
            static std::pair<GLFWmonitor**, int> scanForMonitors() noexcept;
            /// @brief Scans for the monitor that displays the app
            /// @return A VResult type
            ftstd::VResult scanForCurrentMonitor(GLFWwindow* app_window) noexcept;
            /// @brief Returns the properties of the current monitor
            /// @return The properties of the current monitor
            MonitorProperties& getCurrentProperties();
            /// @brief Returns if the current monitor has been found (or not)
            /// @return A boolean value
            bool foundCurrentMonitor() const;
        };
    } // namespace graphics
} // namespace frametech

#endif /* monitor_h */
