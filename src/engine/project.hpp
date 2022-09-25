//
//  project.hpp
//  FrameTech
//
//  Created by Antonin on 23/09/2022.
//

#pragma once
#ifndef engine_project_h
#define engine_project_h

#include "../version.h"
#include <cstdlib>

namespace Project
{
    /// The name of the engine
    constexpr const char* ENGINE_NAME = "FrameTech Engine";
    /// Major version number of the engine
    constexpr uint8_t const ENGINE_VERSION_MAJOR_NUMBER = 0;
    /// Minor version number of the engine
    constexpr uint8_t const ENGINE_VERSION_MINOR_NUMBER = 1;
    /// Bug fix version number of the engine
    constexpr uint8_t const ENGINE_VERSION_BUGFIX_NUMBER = 0;

    /// The Version object to represent / formalize the version of the application
    const Version ENGINE_VERSION = Version(ENGINE_VERSION_MAJOR_NUMBER,
                                           ENGINE_VERSION_MINOR_NUMBER,
                                           ENGINE_VERSION_BUGFIX_NUMBER);

} // namespace Project

#endif // engine_project_h
