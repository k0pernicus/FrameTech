//
//  camera.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "camera.hpp"
#include "../application.hpp"
#include "../engine/engine.hpp"
#include "../ftstd/debug_tools.h"

glm::vec3 frametech::gameframework::Camera::getTarget() const noexcept
{
    return m_target;
}

void frametech::gameframework::Camera::setTarget(const glm::vec3& new_target) noexcept
{
    m_target = new_target;
}

void frametech::gameframework::Camera::resetTarget() noexcept
{
    m_target = m_original_target;
}

glm::vec3 frametech::gameframework::Camera::getPosition() const noexcept
{
    return m_position;
}

void frametech::gameframework::Camera::resetPosition() noexcept
{
    m_position = m_original_position;
}

void frametech::gameframework::Camera::resetDirectionalVectors() noexcept
{
//    m_pitch = DEFAULT_PITCH;
//    m_yaw = DEFAULT_YAW;
//    m_front = DEFAULT_FRONT_VECTOR;
    m_up = DEFAULT_UP_VECTOR;
}

void frametech::gameframework::Camera::setPosition(const glm::vec3& new_position) noexcept
{
    m_position = new_position;
}

float frametech::gameframework::Camera::getFOV() const noexcept
{
    return m_fov;
}

frametech::gameframework::Camera::Type frametech::gameframework::Camera::getType() noexcept
{
    return m_type;
}

void frametech::gameframework::Camera::setType(const Type new_type) noexcept
{
    m_type = new_type;
}

std::string frametech::gameframework::Camera::getTypeName() const noexcept
{
    switch (m_type)
    {
        case Type::STATIONARY:
            return std::string("STATIONARY");
        case Type::WORLD:
            return std::string("WORLD");
        default:
            return std::string("UNKNOWN");
    }
}

void frametech::gameframework::Camera::handleKeyEvent(frametech::engine::inputs::Key& key) noexcept
{
    switch (key)
    {
        case frametech::engine::inputs::Key::ALT_RIGHT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + RIGHT keys have been hit");
            glm::vec3 left = glm::cross(m_up, m_front);
            const float rotation_degree = 1 * CAMERA_ROTATION_STEP;
            m_up = m_up * cos(rotation_degree) + left * sin(rotation_degree);
        }
        break;
        case frametech::engine::inputs::Key::ALT_DOWN_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + DOWN keys have been hit");
            m_pitch += 1;
            m_target.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_target.y = sin(glm::radians(m_pitch));
            m_target.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_front = glm::normalize(m_target);
        }
        break;
        case frametech::engine::inputs::Key::ALT_LEFT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + LEFT keys have been hit");
            glm::vec3 left = glm::cross(m_up, m_front);
            const float rotation_degree = -1 * CAMERA_ROTATION_STEP;
            m_up = m_up * cos(rotation_degree) + left * sin(rotation_degree);
        }
        break;
        case frametech::engine::inputs::Key::ALT_UP_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + UP keys have been hit");
            m_pitch -= 1;
            m_target.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_target.y = sin(glm::radians(m_pitch));
            m_target.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_front = glm::normalize(m_target);
        }
        break;
        case frametech::engine::inputs::Key::RIGHT:
        {
            Log("[CAMERA OBJECT] RIGHT key has been hit");
            m_position += glm::normalize(glm::cross(m_front, m_up)) * CAMERA_MOVE_STEP;
        }
        break;
        case frametech::engine::inputs::Key::DOWN:
        {
            Log("[CAMERA OBJECT] DOWN key has been hit");
            m_position -= CAMERA_MOVE_STEP * m_front;
        }
        break;
        case frametech::engine::inputs::Key::LEFT:
        {
            Log("[CAMERA OBJECT] LEFT key has been hit");
            m_position -= glm::normalize(glm::cross(m_front, m_up)) * CAMERA_MOVE_STEP;
        }
        break;
        case frametech::engine::inputs::Key::UP:
        {
            Log("[CAMERA OBJECT] UP key has been hit");
            m_position += CAMERA_MOVE_STEP * m_front;
        }
        break;
        default:
            LogW("[CAMERA OBJECT] Unknown key with id %d");
            break;
    }
}

void frametech::gameframework::Camera::handleMouseEvent(std::tuple<float, float>& mouse_positions) noexcept {
    
    float x_pos = std::get<0>(mouse_positions);
    float y_pos = std::get<1>(mouse_positions);
    if (m_first_cursor_pos) {
        m_last_x_cursor = x_pos;
        m_last_y_cursor = y_pos;
        m_first_cursor_pos = false;
    }
    
    float x_offset = x_pos - m_last_x_cursor;
    float y_offset = m_last_y_cursor - y_pos;
    
    m_last_x_cursor = x_pos;
    m_last_y_cursor = y_pos;
    
    x_offset *= CAMERA_ROTATION_STEP;
    y_offset *= CAMERA_ROTATION_STEP;
    
    m_yaw += x_offset;
    m_pitch += y_offset;
        
    // Usefull for FPS camera only ?
//    if (m_pitch > 89.0f)  m_pitch =  89.0f;
//    if (m_pitch < -89.0f) m_pitch = -89.0f;
    
    m_target.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_target.y = sin(glm::radians(m_pitch));
    m_target.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(m_target);
}
