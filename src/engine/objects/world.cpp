//
//  world.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "world.hpp"
#include "../ftstd/debug_tools.h"

frametech::engine::MovableInterface* frametech::engine::World::getSelectedObject() noexcept
{
    return m_selected_object;
}

void frametech::engine::World::setSelectedObject(frametech::engine::MovableInterface* object) noexcept
{
    if (nullptr == object)
    {
        LogW("tried to set a NULL object as selected editor object, forbidden");
        return;
    }
    m_selected_object = object;
}

frametech::engine::Camera& frametech::engine::World::getMainCamera() noexcept
{
    return m_main_camera;
}

void frametech::engine::World::init() noexcept
{
    // The default selected object of the world is the main camera
    m_selected_object = static_cast<frametech::engine::MovableInterface*>(&m_main_camera);
}
