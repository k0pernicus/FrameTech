//
//  ubo.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "ubo.hpp"
#include "../engine.hpp"

constexpr float FAR = 100.0f;

// TODO: this should not be an updateUBO function, but an updateTransform function,
// maybe associated to an object...
// Also, this should not return an UniformBufferObject structure but a ModelViewProjection data structure
UniformBufferObject frametech::graphics::updateUBO(
	const frametech::graphics::Transformation targeted_ubo,
	const float delta_time,
	const uint32_t window_height,
	const uint32_t window_width
) noexcept {
	// Compute the view of the world
	glm::vec3 eye = frametech::Engine::getInstance()->m_camera.getPosition();
	glm::vec3 lookat = eye + glm::vec3(0.0f, 0.0f, -1.0f); 
	if (frametech::Engine::getInstance()->m_camera.getType() == frametech::engine::Camera::Type::STATIONARY) {
		lookat = frametech::Engine::getInstance()->m_camera.getDirection();
	}
	glm::vec3 up = glm::vec3(0, 1, 0); // Front view
	glm::mat4 view = glm::lookAt(eye, lookat, up);

	const float fov = frametech::Engine::getInstance()->m_camera.getFOV();

	switch (targeted_ubo) {
	case frametech::graphics::Transformation::Constant:
		return UniformBufferObject{
				.model = glm::mat4(1.0f),
				.view = view,
				.projection = glm::perspective(glm::radians(fov), window_width / (float)window_height, 0.1f, FAR),
		};
	case frametech::graphics::Transformation::Rotate:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = view,
				.projection = glm::perspective(glm::radians(fov), window_width / (float)window_height, 0.1f, FAR),
		};
	case frametech::graphics::Transformation::RotateAndScale:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = view,
				.projection = glm::perspective(glm::radians(fov) * cos(delta_time), window_width / (float)window_height, 0.1f, FAR),
		};
	default:
		LogW("updateUBO: unknown UBO enum found: %d", targeted_ubo);
		return UniformBufferObject{
				.model = glm::mat4(1.0f),
				.view = view,
				.projection = glm::mat4(1.0f),
		};
	}
}
