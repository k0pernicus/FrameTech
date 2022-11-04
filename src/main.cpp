//
//  main.cpp
//  FrameTech
//
//  Created by Antonin on 18/09/2022.
//

#define GLFW_INCLUDE_VULKAN
#include "application.hpp"
#include "engine/project.hpp"
#include "ftstd/arg_parse.h"
#include "ftstd/debug_tools.h"
#include "project.hpp"
#include <GLFW/glfw3.h>
#include <stdio.h>

int main(int argc, const char* argv[])
{
    ArgParse arg_parse = ArgParse(argc, argv);
    {
        char c_version[18];
        Project::APPLICATION_VERSION.toString(c_version);
        Log("Application '%s' (version %s)", Project::APPLICATION_NAME, c_version);
    }
    {
        char c_version[18];
        Project::ENGINE_VERSION.toString(c_version);
        Log("Engine '%s' (version: %s)", Project::ENGINE_NAME, c_version);
    }
    std::unique_ptr<FrameTech::Application> app(FrameTech::Application::getInstance(Project::APPLICATION_NAME));
#ifdef ENABLE_EXCEPTIONS
    Log("/!\\ Exceptions are enabled /!\\");
    try
    {
        app->initWindow();
        app->initEngine();
        app->run();
    }
    catch (const std::exception& e)
    {
        LogE("Exception running %s: %s", Project::APPLICATION_NAME, e.what());
        return EXIT_FAILURE;
    }
#else
    app->initWindow();
    app->initEngine();
    app->run();
#endif
    return EXIT_SUCCESS;
}
