//
//  camera.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "camera.hpp"
#include "../engine/engine.hpp"
#include "../ftstd/debug_tools.h"

glm::vec3 frametech::gameframework::Camera::getDirection() const noexcept
{
    return m_direction;
}

void frametech::gameframework::Camera::setDirection(const glm::vec3& new_direction) noexcept
{
    m_direction = new_direction;
}

void frametech::gameframework::Camera::resetDirection() noexcept
{
    m_direction = m_original_direction;
}

glm::vec3 frametech::gameframework::Camera::getPosition() const noexcept
{
    return m_position;
}

void frametech::gameframework::Camera::resetPosition() noexcept
{
    m_position = m_original_position;
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
    const glm::vec3 camera_direction = frametech::Engine::getInstance()->m_world.getMainCamera().getDirection();
    const glm::vec3 camera_position = frametech::Engine::getInstance()->m_world.getMainCamera().getPosition();
    switch (key)
    {
        case frametech::engine::inputs::Key::ALT_RIGHT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + RIGHT keys have been hit");
            m_direction = glm::vec3(camera_direction.x + CAMERA_ROTATION_STEP, camera_direction.y, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_DOWN_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + DOWN keys have been hit");
            m_direction = glm::vec3(camera_direction.x, camera_direction.y + CAMERA_ROTATION_STEP, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_LEFT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + LEFT keys have been hit");
            m_direction = glm::vec3(camera_direction.x - CAMERA_ROTATION_STEP, camera_direction.y, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_UP_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + UP keys have been hit");
            m_direction = glm::vec3(camera_direction.x, camera_direction.y - CAMERA_ROTATION_STEP, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::RIGHT:
        {
            Log("[CAMERA OBJECT] RIGHT key has been hit");
            m_position = glm::vec3(camera_position.x + CAMERA_MOVE_STEP, camera_position.y, camera_position.z);
            m_direction = glm::vec3(camera_direction.x + CAMERA_MOVE_STEP, camera_direction.y, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::DOWN:
        {
            Log("[CAMERA OBJECT] DOWN key has been hit");
            m_position = glm::vec3(camera_position.x, camera_position.y, camera_position.z + CAMERA_MOVE_STEP);
            m_direction = glm::vec3(camera_direction.x, camera_direction.y, camera_direction.z + CAMERA_MOVE_STEP);
        }
        break;
        case frametech::engine::inputs::Key::LEFT:
        {
            Log("[CAMERA OBJECT] LEFT key has been hit");
            m_position = glm::vec3(camera_position.x - CAMERA_MOVE_STEP, camera_position.y, camera_position.z);
            m_direction = glm::vec3(camera_direction.x - CAMERA_MOVE_STEP, camera_direction.y, camera_direction.z);
        }
        break;
        case frametech::engine::inputs::Key::UP:
        {
            Log("[CAMERA OBJECT] UP key has been hit");
            m_position = glm::vec3(camera_position.x, camera_position.y, camera_position.z - CAMERA_MOVE_STEP);
            m_direction = glm::vec3(camera_direction.x, camera_direction.y, camera_direction.z - CAMERA_MOVE_STEP);
        }
        break;
        default:
            LogW("[CAMERA OBJECT] Unknown key with id %d");
            break;
    }
}
