#version 450

layout (binding = 0) uniform UniformBuffer {
    mat4 model; // glm::mat4
    mat4 view;  // glm::mat4
    mat4 proj;  // glm::mat4
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
