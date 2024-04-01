#version 330 core

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec2 worldUV;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(worldPos, 1.0);
}