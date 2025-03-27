#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D uMaterialDiffuse;

void main() {
    FragColor = vec4(vec3(texture(uMaterialDiffuse, TexCoords)), 1.0);
}
