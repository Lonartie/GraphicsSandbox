#version 330 core

in vec2 uv;

out vec4 worldColor;

uniform sampler2D albedo;
uniform sampler2D normal;

void applyAlbedo();
void applyNormal();

void main() {
    // default color
    worldColor = vec4(1, 0, 1, 1);
    applyAlbedo();
    applyNormal();
}

void applyAlbedo() {
    if (textureSize(albedo, 0).xy == ivec2(1,1)) return;
    worldColor = texture(albedo, uv);
}

void applyNormal() {
    if (textureSize(normal, 0).xy == ivec2(1,1)) return;
    vec3 normalVec = normalize(texture(normal, uv).xyz);
    vec3 lightDir = normalize(vec3(1, 0, 1));
    float intensity = dot(normalVec, lightDir);
    worldColor = vec4(worldColor.rgb * intensity, worldColor.a);
}