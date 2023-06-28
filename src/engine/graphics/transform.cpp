//
//  transform.cpp
//  FrameTech
//
//  Created by Antonin on 06/05/2023.
//

#include "transform.hpp"
#include "../application.hpp"
#include "../engine.hpp"

constexpr float FAR = 100.0f;

ModelViewProjection frametech::graphics::computeTransform(
    const frametech::graphics::Transformation transformation_id,
    const float delta_time,
    const uint32_t window_height,
    const uint32_t window_width) noexcept
{
    // Compute the view of the world
    const frametech::gameframework::Camera& world_camera = frametech::Application::getInstance("")->getCurrentWorld().getMainCamera();
    glm::vec3 eye = world_camera.getPosition();
    glm::vec3 lookat = world_camera.getDirection();
    glm::vec3 up = glm::vec3(0, 1, 0); // Front view
    glm::mat4 view = glm::lookAt(eye, lookat, up);

    const float fov = world_camera.getFOV();

    // TODO: check + integrate quaternions
    switch (transformation_id)
    {
        case frametech::graphics::Transformation::Constant:
            return ModelViewProjection{
                .model = glm::mat4(1.0f),
                .view = view,
                .projection = glm::perspective(glm::radians(fov), window_width / (float)window_height, 0.1f, FAR),
            };
        case frametech::graphics::Transformation::Rotate:
            return ModelViewProjection{
                .model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .view = view,
                .projection = glm::perspective(glm::radians(fov), window_width / (float)window_height, 0.1f, FAR),
            };
        case frametech::graphics::Transformation::RotateAndScale:
            return ModelViewProjection{
                .model = glm::rotate(glm::mat4(1.0f) * delta_time, delta_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .view = view,
                .projection = glm::perspective(glm::radians(fov) * cos(delta_time), window_width / (float)window_height, 0.1f, FAR),
            };
        default:
            LogW("computeTransform: unknown Transformation enum found: %d", transformation_id);
            return ModelViewProjection{
                .model = glm::mat4(1.0f),
                .view = view,
                .projection = glm::mat4(1.0f),
            };
    }
}
