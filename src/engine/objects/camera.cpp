//
//  camera.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "camera.hpp"

glm::vec3 frametech::engine::Camera::getDirection() const noexcept {
	return m_direction;
}

void frametech::engine::Camera::setDirection(const glm::vec3& new_direction) noexcept {
	m_direction = new_direction;
}

void frametech::engine::Camera::resetDirection() noexcept {
	m_direction = m_original_direction;
}
