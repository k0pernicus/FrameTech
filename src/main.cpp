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

#ifdef ENABLE_EXCEPTIONS
#define TOML_EXCEPTIONS 1
#else
#define TOML_EXCEPTIONS 0
#endif

Project::GameProjectSettings* GAME_APPLICATION_SETTINGS = new Project::GameProjectSettings();

int main(int argc, const char* argv[])
{
    auto arg_parse = ftstd::ArgParse(argc, argv);
    if (auto result = GAME_APPLICATION_SETTINGS->loadFrom(Project::DEFAULT_GAME_DESC_FILENAME); result.IsError())
    {
        LogE("Error reading the game configuration at %s", Project::DEFAULT_GAME_DESC_FILENAME);
        return EXIT_FAILURE;
    }
    else
    {
        Log("Successfully readed %s", Project::DEFAULT_GAME_DESC_FILENAME);
    }
    {
        GAME_APPLICATION_SETTINGS->version.toString(S_APP_VERSION);
        Log("Application '%s' (version %s)", GAME_APPLICATION_SETTINGS->name.c_str(), S_APP_VERSION);
    }
    {
        Project::ENGINE_VERSION.toString(S_ENGINE_VERSION);
        Log("Engine '%s' (version: %s)", Project::ENGINE_NAME, S_ENGINE_VERSION);
    }
    std::unique_ptr<frametech::Application> app(frametech::Application::getInstance(GAME_APPLICATION_SETTINGS->name.c_str()));
#ifdef ENABLE_EXCEPTIONS
    Log("/!\\ Exceptions are enabled /!\\");
    try
#endif
    {
        if (const auto result_code = app->initWindow(); result_code.IsError())
            return EXIT_FAILURE;

        // The following state machine is pretty bad, as we should not initialize the engine, load assets, and
        // then finish to initialize the engine with descriptor sets...
        // We have to do this as descriptor sets require samplers & image objects to be init, but game assets
        // require some renderer parts to allocate & init textures and samplers...
        // TODO: find a good solution here to solve this very bad issue !

        app->initEngine();
        app->loadGameAssets();
        app->initDescriptorSets();
        app->run();
    }
#ifdef ENABLE_EXCEPTIONS
    catch (const std::exception& e)
    {
        LogE("Exception running %s: %s", Project::DEFAULT_APPLICATION_NAME, e.what());
        return EXIT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}
