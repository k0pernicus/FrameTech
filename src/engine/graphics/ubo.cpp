//
//  ubo.hpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "ubo.hpp"

UniformBufferObject frametech::graphics::updateUBO(
	const frametech::graphics::Transformation targeted_ubo,
	const float delta_time,
	const uint32_t window_height,
	const uint32_t window_width
) noexcept {
	// ubo.projection[1][1] *= -1;
	switch (targeted_ubo) {
	case frametech::graphics::Transformation::Constant:
		return UniformBufferObject{
				.model = glm::mat4(1.0f),
				.view = glm::mat4(1.0f),
				.projection = glm::mat4(1.0f),
		};
	case frametech::graphics::Transformation::Rotate:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = glm::mat4(1.0f),
				.projection = glm::mat4(1.0f),
		};
	case frametech::graphics::Transformation::RotateAndScale:
		return UniformBufferObject{
				.model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.view = glm::lookAt(glm::vec3(4.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
				.projection = glm::perspective(glm::radians(45.0f) * cos(delta_time), window_width / (float)window_height, 0.1f, 10.0f),
		};
	default:
		LogW("updateUBO: unknown UBO enum found: %d", targeted_ubo);
		return UniformBufferObject{
				.model = glm::mat4(1.0f),
				.view = glm::mat4(1.0f),
				.projection = glm::mat4(1.0f),
		};
	}
}
