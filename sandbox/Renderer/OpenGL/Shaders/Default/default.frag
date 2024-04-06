#version 330 core

out vec4 worldColor;
uniform vec4 solidColor;

void main() {
    worldColor = solidColor;
}