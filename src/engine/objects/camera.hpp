//
//  camera.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#pragma once
#ifndef _camera_hpp
#define _camera_hpp

#include <glm/glm.hpp>
#include <string>

/// @brief Default field of view - commonly used value of 60 degrees
constexpr float DEFAULT_FOV = 60.0f;
constexpr glm::vec3 DEFAULT_DIRECTION = glm::vec3(0.0f);
constexpr glm::vec3 DEFAULT_POSITION = glm::vec3(0.0f, 0.0f, 3.0f);

namespace frametech {
	namespace engine {
		class Camera {
		public:
			enum Type {
				STATIONARY,
				WORLD,
			};
		public:
			Camera(const glm::vec3 direction = DEFAULT_DIRECTION, const glm::vec3 position = DEFAULT_POSITION, const Type camera_type = Type::WORLD) :
				m_original_direction(direction), 
				m_direction(direction), 
				m_original_position(position),
				m_position(position),
				m_type(camera_type)
			{};
			glm::vec3 getDirection() const noexcept;
			void resetDirection() noexcept;
			void setDirection(const glm::vec3& new_direction) noexcept;
			glm::vec3 getPosition() const noexcept;
			void resetPosition() noexcept;
			void setPosition(const glm::vec3& new_position) noexcept;
			float getFOV() const noexcept;
			Type getType() noexcept;
			void setType(const Type new_type) noexcept;
			/// @brief Return a tag associated to the current camera type
			/// @return A tag, as a string, associated to the current camera type setting
			std::string getTypeName() const noexcept;
		private:
			glm::vec3 m_original_direction;
			glm::vec3 m_direction;
			glm::vec3 m_original_position;
			glm::vec3 m_position;
			float m_fov = DEFAULT_FOV;
			Type m_type = Type::WORLD;
			// TODO : Camera position
		};
	}
}

#endif // _camera_hpp
