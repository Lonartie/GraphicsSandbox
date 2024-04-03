#version 330 core
#pragma optimize (off)

out vec4 worldColor;
uniform vec4 solidColor;

void main() {
    // default color is white
    worldColor = solidColor;
}