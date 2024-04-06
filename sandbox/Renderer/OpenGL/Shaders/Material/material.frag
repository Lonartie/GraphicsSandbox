#version 330 core

in vec2 uv;

out vec4 worldColor;
uniform sampler2D albedo;

void main() {
    ivec2 texSize = textureSize(albedo, 0);
    vec2 texCoord = uv * vec2(texSize);
    worldColor = texelFetch(albedo, ivec2(texCoord), 0);
}