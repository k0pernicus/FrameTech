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
#include "tomlplusplus/toml.hpp"
#include <cstdlib>
#include <optional>

/// @brief Limits of 240 FPS
constexpr u8 FPS_LIMIT_240 = 240;
/// @brief Limits of 120 FPS
constexpr u8 FPS_LIMIT_120 = 120;
/// @brief Limits of 144 FPS
constexpr u8 FPS_LIMIT_144 = 144;
/// @brief Limits of 60 FPS
constexpr u8 FPS_LIMIT_60 = 60;
/// @brief Limits of 30 FPS
constexpr u8 FPS_LIMIT_30 = 30;
/// @brief No FPS limit
constexpr std::optional<u8> NO_FPS_LIMIT = std::nullopt;

namespace Project
{

    /// @brief The name of the application
    constexpr const char* DEFAULT_APPLICATION_NAME = (char*)"Example";

#ifdef UNSET_FPS_LIMIT
    /// @brief Unlimited FPS
    constexpr std::optional<u8> DEFAULT_APPLICATION_FPS_LIMIT = FPS_LIMIT_60;
#else
    /// @brief Pause / limit the Application to 60 FPS by default
    constexpr std::optional<u8> APPLICATION_FPS_LIMIT = FPS_LIMIT_60;
#endif

    /// @brief Minimum major version number of the Vulkan API
    constexpr u16 const VULKAN_MIN_VERSION_MAJOR = 1;
    /// @brief Minimum minor version number of the Vulkan API
    constexpr u16 const VULKAN_MIN_VERSION_MINOR = 3;
    /// @brief Minimum bug fix version number of the Vulkan API
    constexpr u16 const VULKAN_MIN_VERSION_BUGFIX = 288;

    /// @brief The entry file of the game, that describes all the assets & scripts to use for
    constexpr const char* DEFAULT_GAME_DESC_FILENAME = (char*)"game.toml";

    struct GameProjectSettings
    {
    public:
        /// @brief Game project name
        std::string name;
        /// @brief Targetted FPS
        std::optional<u8> fps_target;
        /// @brief All the asset archives to load
        std::vector<std::string> asset_folders;
        /// @brief Application version
        ftstd::Version version;

        ftstd::VResult loadFrom(const char* const game_settings_filename) noexcept
        {
            toml::table toml_config;
#if defined(ENABLE_EXCEPTIONS)
            try
            {
                toml_config = toml::parse_file(game_settings_filename);
            }
            catch (const toml::parse_error& err)
            {
                LogE("Error loading the project settings at %s", game_settings_filename);
                return ftstd::VResult::Error((char*)"Error loading the project settings");
            }
#else
            toml::parse_result result = toml::parse_file(game_settings_filename);
            if (!result)
            {
                LogE("Error loading the project settings at %s", game_settings_filename);
                return ftstd::VResult::Error((char*)"Error loading the project settings");
            }
            toml_config = std::move(result).table();
#endif
            this->name = toml_config["APPLICATION_NAME"].value_or(DEFAULT_APPLICATION_NAME);
            this->fps_target = toml_config["APPLICATION_FPS_TARGET"].value<u8>();
            if (!this->fps_target.has_value())
                this->fps_target = DEFAULT_APPLICATION_FPS_LIMIT;
            const std::optional<std::string> raw_version = toml_config["APPLICATION_VERSION"].value<std::string>();
            if (!raw_version.has_value())
            {
                return ftstd::VResult::Error((char*)"The game settings file does not contain any application version (REQUIRED)");
            }
            this->version = ftstd::Version(raw_version.value());
            auto assets = toml_config["APPLICATION_DEPENDENCIES"];
            if (toml::array* assets_array = assets.as_array())
            {
                assets_array->for_each([this](auto&& el)
                                       {
                    if constexpr (toml::is_string<decltype(el)>)
                    {
                        Log("Found asset: %s", (*el).c_str());
                        this->asset_folders.emplace_back(*el);
                    } });
            }
            return ftstd::VResult::Ok();
        }
    };

} // namespace Project

extern Project::GameProjectSettings* GAME_APPLICATION_SETTINGS;

#endif // app_project_h
