//
//  movable.hpp
//  FrameTech
//
//  Created by Antonin on 21/05/2023.
//

#pragma once
#ifndef _movable_hpp
#define _movable_hpp

#include "../engine/inputs/inputs.hpp"
#include <glm/glm.hpp>

constexpr float DEFAULT_PITCH = 0.0f;
constexpr float DEFAULT_YAW = -90.0f;
constexpr glm::vec3 DEFAULT_FRONT_VECTOR = glm::vec3(0.0f, 0.0f, -1.0f); // (_, _, z)
constexpr glm::vec3 DEFAULT_UP_VECTOR = glm::vec3(0.0f, 1.0f, 0.0f);     // (_, y, _)

namespace frametech
{
    namespace gameframework
    {
        /// @brief Pure abstract class (interface) to let the developer
        /// easily handle movements (frametech's inputs), for any object
        class MovableInterface
        {
        public:
            /// @brief Handles move(ment) key
            /// @param key A key handled by frametech's inputs
            /// TODO: check if necessary to set, as parameter, if the input needs to propagate to children / parents
            /// or not
            virtual void handleKeyEvent(frametech::engine::inputs::KeyMask& mask) noexcept = 0;
            /// @brief Handles cursor / mouse / touchable event
            /// @param mouse_positions The new positions of the mouse / cursor
            virtual void handleMouseEvent(std::tuple<float, float>& mouse_positions) noexcept = 0;
            
            inline const glm::vec3& getFrontVector() const noexcept { return m_front; }
            inline const glm::vec3& getUpVector() const noexcept { return m_up; }
            
            /// @brief Boolean value to retrieve and compute mouse position events
            bool m_first_cursor_pos = true;
            /// @brief Movement pitch (up / down)
            float m_pitch = DEFAULT_PITCH;
            /// @brief Movement yaw   (left / right)
            float m_yaw = DEFAULT_YAW;
            /// @brief Front vector
            glm::vec3 m_front = DEFAULT_FRONT_VECTOR;
            /// @brief Up vector
            glm::vec3 m_up = DEFAULT_UP_VECTOR;
            /// @brief Right vector
            glm::vec3 m_right = glm::vec3(0.0f, 0.0f, 0.0f);
        };
    } // namespace gameframework
} // namespace frametech

#endif // _movable_hpp
