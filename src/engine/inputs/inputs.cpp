//
//  inputs.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "inputs.hpp"
#include "../engine.hpp"
#include "../ftstd/debug_tools.h"

void frametech::engine::inputs::EventHandler::poll(bool blank) noexcept
{
    if (blank)
        return;
    std::scoped_lock<std::mutex> guard(m_lock);
    if (m_keys.empty())
        return;
    if (nullptr != frametech::Engine::getInstance()->m_world.getSelectedObject())
        frametech::Engine::getInstance()->m_world.getSelectedObject()->handleKeyEvent(m_keys.front());
    m_keys.pop_front();
}

void frametech::engine::inputs::EventHandler::addKey(const Key key) noexcept
{
    std::scoped_lock<std::mutex> guard(m_lock);
    m_keys.push_back(key);
}
