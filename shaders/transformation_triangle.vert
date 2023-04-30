#version 450

layout (binding = 0) uniform UniformBuffer {
    mat4 model;       // glm::mat4
    mat4 view;        // glm::mat4
    mat4 projection;  // glm::mat4
} transformation;

layout (location = 0) in vec2 inPosition;   // Vertex attributes
layout (location = 1) in vec3 inColor;      // Vertex attributes

layout (location = 0) out vec3 outFragment; // Color (fragment)

void main() {
    gl_Position = transformation.projection * transformation.view * transformation.model * vec4(inPosition, 0.0, 1.0);
    outFragment = inColor;
}
