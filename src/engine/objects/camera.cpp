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

glm::vec3 frametech::engine::Camera::getPosition() const noexcept {
	return m_position;
}

void frametech::engine::Camera::resetPosition() noexcept {
	m_position = m_original_position;
}

void frametech::engine::Camera::setPosition(const glm::vec3& new_position) noexcept {
	m_position = new_position;
}

float frametech::engine::Camera::getFOV() const noexcept {
	return m_fov;
}

frametech::engine::Camera::Type frametech::engine::Camera::getType() noexcept {
	return m_type;
}

void frametech::engine::Camera::setType(const Type new_type) noexcept {
	m_type = new_type;
}

std::string frametech::engine::Camera::getTypeName() const noexcept {
	switch (m_type) {
	case Type::STATIONARY: return std::string("STATIONARY");
	case Type::WORLD: return std::string("WORLD");
	default: return std::string("UNKNOWN");
	}
}
