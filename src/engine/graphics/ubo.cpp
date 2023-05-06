//
//  ubo.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "ubo.hpp"
#include "../engine.hpp"

UniformBufferObject frametech::graphics::updateUBO(
	const frametech::graphics::Transformation targeted_ubo,
	const float delta_time,
	const uint32_t window_height,
	const uint32_t window_width
) noexcept {
	// Compute the view of the world
	// The camera is STATIONARY for the moment - should be defined elsewhere I think
	glm::vec3 eye = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 lookat = frametech::Engine::getInstance()->m_camera.getDirection();
	glm::vec3 up = glm::vec3(0, 1, 0); // Front view
	glm::mat4 view = glm::lookAt(eye, lookat, up);

	switch (targeted_ubo) {
	case frametech::graphics::Transformation::Constant:
		return UniformBufferObject{
				.model = glm::mat4(1.0f),
				.view = view,
				.projection = glm::perspective(glm::radians(45.0f), window_width / (float)window_height, 0.1f, 10.0f),
		};
	case frametech::graphics::Transformation::Rotate:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = view,
				.projection = glm::perspective(glm::radians(45.0f), window_width / (float)window_height, 0.1f, 10.0f),
		};
	case frametech::graphics::Transformation::RotateAndScale:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = view,
				.projection = glm::perspective(glm::radians(45.0f) * cos(delta_time), window_width / (float)window_height, 0.1f, 10.0f),
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
