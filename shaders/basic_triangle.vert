#version 450

layout (location = 0) in vec3 inPosition;   // Vertex position
layout (location = 1) in vec3 inColor;      // Vertex color
layout (location = 2) in vec2 inTexCoord;   // Vertex color

layout (location = 0) out vec3 outFragment; // Color (fragment)
layout (location = 1) out vec2 outTexCoord; // Texture coordinates

void main() {
    gl_Position = vec4(inPosition, 1.0);
    outFragment = inColor;
    outTexCoord = inTexCoord;
}
