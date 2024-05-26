//
//  camera.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _camera_hpp
#define _camera_hpp

#include "movable.hpp"
#include <glm/glm.hpp>
#include <string>

/// @brief Default field of view - commonly used value of 60 degrees
constexpr float DEFAULT_FOV = 65.0f;
constexpr glm::vec3 DEFAULT_TARGET = glm::vec3(0.0, 0.0, 0.0f);
constexpr glm::vec3 DEFAULT_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
constexpr float CAMERA_ROTATION_STEP = 0.05f;
constexpr float CAMERA_MOVE_STEP = 0.05f;

namespace frametech
{
    namespace gameframework
    {
    
        class Camera : public frametech::gameframework::MovableInterface
        {
        public:
            Camera(const glm::vec3 target = DEFAULT_TARGET, glm::vec3 position = DEFAULT_POSITION) : m_original_target(target),
                  m_target(target),
                  m_original_position(position),
                  m_position(position){};
            const glm::vec3& getTarget() const noexcept;
            void resetTarget() noexcept;
            void setTarget(const glm::vec3& new_target) noexcept;
            const glm::vec3& getPosition() const noexcept;
            void resetPosition() noexcept;
            void resetDirectionalVectors() noexcept;
            void setPosition(const glm::vec3& new_position) noexcept;
            float getFOV() const noexcept;
            const glm::vec3& getDirection() const noexcept;
            void handleKeyEvent(frametech::engine::inputs::KeyMask& mask) noexcept override;
            void handleMouseEvent(std::tuple<float, float>& mouse_positions) noexcept override;

        private:
            glm::vec3 m_original_target;
            glm::vec3 m_target;
            glm::vec3 m_original_position;
            glm::vec3 m_position;
            glm::vec3 m_direction;
            float m_last_x_cursor = 0;
            float m_last_y_cursor = 0;
            float m_fov = DEFAULT_FOV;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _camera_hpp
