//
//  world.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "world.hpp"
#include "../ftstd/debug_tools.h"

frametech::gameframework::MovableInterface* frametech::gameframework::World::getSelectedObject() noexcept
{
    return m_selected_object;
}

void frametech::gameframework::World::setSelectedObject(frametech::gameframework::MovableInterface* object) noexcept
{
    if (nullptr == object)
    {
        LogW("tried to set a NULL object as selected editor object, forbidden");
        return;
    }
    m_selected_object = object;
}

frametech::gameframework::Camera& frametech::gameframework::World::getMainCamera() noexcept
{
    return m_main_camera;
}

void frametech::gameframework::World::setup() noexcept
{
    // The default selected object of the world is the main camera
    m_selected_object = static_cast<frametech::gameframework::MovableInterface*>(&m_main_camera);
    m_has_been_setup = true;
}

void frametech::gameframework::World::clean() noexcept
{
    if (!m_textures_cache.empty())
    {
        for (const auto& [key, value] : m_textures_cache)
        {
            delete value;
        }
        m_textures_cache.clear();
    }
}

bool frametech::gameframework::World::hasBeenSetup() const noexcept
{
    return m_has_been_setup;
}
