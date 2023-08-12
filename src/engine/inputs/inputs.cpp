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

void frametech::engine::inputs::CursorHandler::poll(bool blank) noexcept
{
    if (blank)
        return;
    ftstd::mutex::ScopedMutex guard(&m_lock, (char*)"EventHandler::poll");
    if (m_positions.empty())
        return;
    // Handle
    std::tuple<double, double> cursor_position = m_positions.front();
    // Check if all positive values,
    // Negative values mean the mouse is outside the current window, we don't want to process that
    if (std::get<0>(cursor_position) >= 0.0f && std::get<1>(cursor_position) >= 0.0f) {
        Log("Cursor moved : x is %f and y is %f", std::get<0>(cursor_position), std::get<1>(cursor_position));
    }
    m_positions.pop_front();
}

void frametech::engine::inputs::CursorHandler::addMove(const double xpos, const double ypos) noexcept
{
    ftstd::mutex::ScopedMutex guard(&m_lock, (char*)"CursorHandler::addMove");
    m_positions.push_back(std::tuple<double, double>(xpos, ypos));
}
