//
//  inputs.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "inputs.hpp"
#include "../application.hpp"
#include "../engine.hpp"
#include "../ftstd/debug_tools.h"
#include "../ftstd/mutex.hpp"

void frametech::engine::inputs::EventHandler::poll(bool blank) noexcept
{
    if (blank)
        return;
    ftstd::mutex::ScopedMutex guard(&m_lock, (char*)"EventHandler::poll");
    if (m_keys.empty())
        return;
    auto selected_object = frametech::Application::getInstance("")->getCurrentWorld().getSelectedObject();
    if (nullptr != selected_object)
        selected_object->handleKeyEvent(m_keys.front());
    m_keys.pop_front();
}

void frametech::engine::inputs::EventHandler::addKey(const Key key) noexcept
{
    ftstd::mutex::ScopedMutex guard(&m_lock, (char*)"EventHandler::addKey");
    m_keys.push_back(key);
}
