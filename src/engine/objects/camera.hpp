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

namespace frametech {
	namespace engine {
		class Camera {
		public:
			Camera(glm::vec3 direction = glm::vec3(0.0f)) : m_original_direction(direction), m_direction(direction) {};
			glm::vec3 getDirection() const noexcept;
			void resetDirection() noexcept;
			void setDirection(const glm::vec3& new_direction) noexcept;
		private:
			glm::vec3 m_original_direction;
			glm::vec3 m_direction;
			// TODO : Camera position
		};
	}
}

#endif // _camera_hpp
