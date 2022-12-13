//
//  main.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "application.hpp"
#include "engine/project.hpp"
#include "ftstd/arg_parse.h"
#include "ftstd/debug_tools.h"
#include "project.hpp"
#include <GLFW/glfw3.h>
#include <stdio.h>

/// @brief Application version, as a string
char S_APP_VERSION[18];

/// @brief Engine version, as a string
char S_ENGINE_VERSION[18];

int main(int argc, const char* argv[])
{
    auto arg_parse = ftstd::ArgParse(argc, argv);
    {
        // char c_version[18];
        Project::APPLICATION_VERSION.toString(S_APP_VERSION);
        Log("Application '%s' (version %s)", Project::APPLICATION_NAME, S_APP_VERSION);
    }
    {
        // char c_version[18];
        Project::ENGINE_VERSION.toString(S_ENGINE_VERSION);
        Log("Engine '%s' (version: %s)", Project::ENGINE_NAME, S_ENGINE_VERSION);
    }
    std::unique_ptr<frametech::Application> app(frametech::Application::getInstance(Project::APPLICATION_NAME));
#ifdef ENABLE_EXCEPTIONS
    Log("/!\\ Exceptions are enabled /!\\");
    try
    {
        if (const auto result_code = app->initWindow(); result_code.IsError())
            return EXIT_FAILURE;
        app->initEngine();
        app->run();
    }
    catch (const std::exception& e)
    {
        LogE("Exception running %s: %s", Project::APPLICATION_NAME, e.what());
        return EXIT_FAILURE;
    }
#else
    if (const auto result_code = app->initWindow(); result_code.IsError())
        return EXIT_FAILURE;
    app->initEngine();
    app->run();
#endif
    return EXIT_SUCCESS;
}
