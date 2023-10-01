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
    const frametech::gameframework::Camera& world_camera = frametech::Application::getInstance("")->getCurrentWorld().getMainCamera();
    const glm::vec3 camera_target = world_camera.getTarget();
    const glm::vec3 camera_position = world_camera.getPosition();
    switch (key)
    {
        case frametech::engine::inputs::Key::ALT_RIGHT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + RIGHT keys have been hit");
            m_target = glm::vec3(camera_target.x + cos(glm::radians(CAMERA_ROTATION_STEP)), camera_target.y, camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_DOWN_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + DOWN keys have been hit");
            m_target = glm::vec3(camera_target.x, camera_target.y + cos(glm::radians(CAMERA_ROTATION_STEP)), camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_LEFT_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + LEFT keys have been hit");
            m_target = glm::vec3(camera_target.x - cos(glm::radians(CAMERA_ROTATION_STEP)), camera_target.y, camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::ALT_UP_COMBINED:
        {
            Log("[CAMERA OBJECT] ALT + UP keys have been hit");
            m_target = glm::vec3(camera_target.x, camera_target.y - cos(glm::radians(CAMERA_ROTATION_STEP)), camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::RIGHT:
        {
            Log("[CAMERA OBJECT] RIGHT key has been hit");
            m_position = glm::vec3(camera_position.x + CAMERA_MOVE_STEP, camera_position.y, camera_position.z);
            m_target = glm::vec3(camera_target.x + CAMERA_MOVE_STEP, camera_target.y, camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::DOWN:
        {
            Log("[CAMERA OBJECT] DOWN key has been hit");
            m_position = glm::vec3(camera_position.x, camera_position.y, camera_position.z + CAMERA_MOVE_STEP);
            m_target = glm::vec3(camera_target.x, camera_target.y, camera_target.z + CAMERA_MOVE_STEP);
        }
        break;
        case frametech::engine::inputs::Key::LEFT:
        {
            Log("[CAMERA OBJECT] LEFT key has been hit");
            m_position = glm::vec3(camera_position.x - CAMERA_MOVE_STEP, camera_position.y, camera_position.z);
            m_target = glm::vec3(camera_target.x - CAMERA_MOVE_STEP, camera_target.y, camera_target.z);
        }
        break;
        case frametech::engine::inputs::Key::UP:
        {
            Log("[CAMERA OBJECT] UP key has been hit");
            m_position = glm::vec3(camera_position.x, camera_position.y, camera_position.z - CAMERA_MOVE_STEP);
            m_target = glm::vec3(camera_target.x, camera_target.y, camera_target.z - CAMERA_MOVE_STEP);
        }
        break;
        default:
            LogW("[CAMERA OBJECT] Unknown key with id %d");
            break;
    }
}
