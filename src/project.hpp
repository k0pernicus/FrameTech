//
//  project.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef app_project_h
#define app_project_h

#include "version.h"
#include <cstdlib>

namespace Project
{

    /// The name of the application
    constexpr const char* APPLICATION_NAME = "Example";
    /// Major version number of the application
    constexpr uint8_t const APPLICATION_VERSION_MAJOR_NUMBER = 0;
    /// Minor version number of the application
    constexpr uint8_t const APPLICATION_VERSION_MINOR_NUMBER = 1;
    /// Bug fix version number of the application
    constexpr uint8_t const APPLICATION_VERSION_BUGFIX_NUMBER = 0;

    /// The Version object to represent / formalize the version of the application
    const Version APPLICATION_VERSION = Version(APPLICATION_VERSION_MAJOR_NUMBER,
                                                APPLICATION_VERSION_MINOR_NUMBER,
                                                APPLICATION_VERSION_BUGFIX_NUMBER);

} // namespace Project

#endif // app_project_h
