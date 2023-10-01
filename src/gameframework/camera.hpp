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
constexpr float DEFAULT_FOV = 80.0f;
constexpr glm::vec3 DEFAULT_TARGET = glm::vec3(0.0, 0.0, 0.0f);
constexpr glm::vec3 DEFAULT_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);
constexpr double CAMERA_ROTATION_STEP = 5;
constexpr double CAMERA_MOVE_STEP = 0.15;

namespace frametech
{
    namespace gameframework
    {
        class Camera : public frametech::gameframework::MovableInterface
        {
        public:
            enum Type
            {
                STATIONARY,
                WORLD,
            };

        public:
            Camera(const glm::vec3 target = DEFAULT_TARGET, glm::vec3 position = DEFAULT_POSITION, const Type camera_type = Type::WORLD) : m_original_target(target),
                  m_target(target),
                  m_position(position),
                  m_original_position(position),
                  m_type(camera_type){};
            glm::vec3 getTarget() const noexcept;
            void resetTarget() noexcept;
            void setTarget(const glm::vec3& new_target) noexcept;
            glm::vec3 getPosition() const noexcept;
            void resetPosition() noexcept;
            void setPosition(const glm::vec3& new_position) noexcept;
            float getFOV() const noexcept;
            Type getType() noexcept;
            void setType(const Type new_type) noexcept;
            /// @brief Return a tag associated to the current camera type
            /// @return A tag, as a string, associated to the current camera type setting
            std::string getTypeName() const noexcept;
            void handleKeyEvent(frametech::engine::inputs::Key& key) noexcept override;

        private:
            glm::vec3 m_original_target;
            glm::vec3 m_target;
            glm::vec3 m_original_position;
            glm::vec3 m_position;
            float m_fov = DEFAULT_FOV;
            /// @brief States what is the current type of the camera (check for 'Type' enum)
            Type m_type = Type::WORLD;
        };
    } // namespace gameframework
} // namespace frametech

#endif // _camera_hpp
