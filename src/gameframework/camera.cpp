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

const glm::vec3& frametech::gameframework::Camera::getTarget() const noexcept
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

const glm::vec3& frametech::gameframework::Camera::getPosition() const noexcept
{
    return m_position;
}

void frametech::gameframework::Camera::resetPosition() noexcept
{
    m_position = m_original_position;
}

const glm::vec3& frametech::gameframework::Camera::getDirection() const noexcept 
{
    return m_direction;
}

void frametech::gameframework::Camera::resetDirectionalVectors() noexcept
{
    m_pitch = DEFAULT_PITCH;
    m_yaw = DEFAULT_YAW;
    m_front = DEFAULT_FRONT_VECTOR;
    m_up = DEFAULT_UP_VECTOR;
    m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
}

void frametech::gameframework::Camera::setPosition(const glm::vec3& new_position) noexcept
{
    m_position = new_position;
}

float frametech::gameframework::Camera::getFOV() const noexcept
{
    return m_fov;
}

void frametech::gameframework::Camera::handleKeyEvent(frametech::engine::inputs::KeyMask& mask) noexcept
{
    switch (mask)
    {
        case frametech::engine::inputs::KeyMask::ALT_RIGHT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + RIGHT keys have been hit");
            const glm::f32 roll_factor = glm::radians(90.0f);
            glm::mat4 roll = glm::rotate(glm::mat4(1.0f), glm::radians(roll_factor), m_front);
            m_up = glm::mat3(roll) * m_up;
        }
        break;
        case frametech::engine::inputs::KeyMask::ALT_LEFT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + LEFT keys have been hit");
            const glm::f32 roll_factor = glm::radians(-90.0f);
            glm::mat4 roll = glm::rotate(glm::mat4(1.0f), glm::radians(roll_factor), m_front);
            m_up = glm::mat3(roll) * m_up;
        }
        break;
        case frametech::engine::inputs::KeyMask::ALT_DOWN_COMBINED: break;
        case frametech::engine::inputs::KeyMask::ALT_UP_COMBINED: break;
        case frametech::engine::inputs::KeyMask::RIGHT:
        {
            Log("[CAMERA OBJECT] RIGHT key has been hit");
            m_position += glm::normalize(glm::cross(m_front, m_up)) * CAMERA_MOVE_STEP;
            m_direction = glm::normalize(m_position - m_target);
            m_right = glm::normalize(glm::cross(m_up, m_direction));
            m_up = glm::normalize(glm::cross(m_direction, m_right));
        }
        break;
        case frametech::engine::inputs::KeyMask::DOWN:
        {
            Log("[CAMERA OBJECT] DOWN key has been hit");
            m_position -= CAMERA_MOVE_STEP * m_front;
            m_direction = glm::normalize(m_position - m_target);
            m_right = glm::normalize(glm::cross(m_up, m_direction));
            m_up = glm::normalize(glm::cross(m_direction, m_right));
        }
        break;
        case frametech::engine::inputs::KeyMask::LEFT:
        {
            Log("[CAMERA OBJECT] LEFT key has been hit");
            m_position -= glm::normalize(glm::cross(m_front, m_up)) * CAMERA_MOVE_STEP;
            m_direction = glm::normalize(m_position - m_target);
            m_right = glm::normalize(glm::cross(m_up, m_direction));
            m_up = glm::normalize(glm::cross(m_direction, m_right));
        }
        break;
        case frametech::engine::inputs::KeyMask::UP:
        {
            Log("[CAMERA OBJECT] UP key has been hit");
            m_position += CAMERA_MOVE_STEP * m_front;
            m_direction = glm::normalize(m_position - m_target);
            m_right = glm::normalize(glm::cross(m_up, m_direction));
            m_up = glm::normalize(glm::cross(m_direction, m_right));
        }
        break;
        default:
            LogW("[CAMERA OBJECT] Unknown key with id %d");
            break;
    }
}

void frametech::gameframework::Camera::handleMouseEvent(std::tuple<float, float>& mouse_positions) noexcept 
{    
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
    
    const float sensitivity = 0.1f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    m_yaw   += x_offset;
    m_pitch += y_offset;

    if (m_pitch > 89.0f)  m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
    
    m_target.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_target.y = sin(glm::radians(m_pitch));
    m_target.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(m_target);
}
