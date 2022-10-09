//
//  project.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef app_project_h
#define app_project_h

#include "ftstd/version.h"
#include <cstdlib>

namespace Project
{

    /// @brief The name of the application
    constexpr const char* APPLICATION_NAME = "Example";
    /// @brief Major version number of the application
    constexpr uint8_t const APPLICATION_VERSION_MAJOR_NUMBER = 0;
    /// @brief Minor version number of the application
    constexpr uint8_t const APPLICATION_VERSION_MINOR_NUMBER = 1;
    /// @brief Bug fix version number of the application
    constexpr uint8_t const APPLICATION_VERSION_BUGFIX_NUMBER = 0;

    /// @brief The Version object to represent / formalize the version of the application
    const Version APPLICATION_VERSION = Version(APPLICATION_VERSION_MAJOR_NUMBER,
                                                APPLICATION_VERSION_MINOR_NUMBER,
                                                APPLICATION_VERSION_BUGFIX_NUMBER);

} // namespace Project

#endif // app_project_h
