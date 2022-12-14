#version 450

layout(binding = 0) uniform TestUBO {
    float test;
} testUBO;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 colour;

layout(location = 0) out vec3 fragColour;

void main() {
    // fragColour = colour;
    fragColour = vec3(testUBO.test);
    gl_Position = vec4(position, 0.0, 1.0);
}