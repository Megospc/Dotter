#version 330 core

in vec2 fPos;
in vec3 fBaseColor;

out vec4 fColor;

void main() {
    float l2 = fPos.x*fPos.x+fPos.y*fPos.y;

    fColor = vec4(fBaseColor, l2 > 1.0 ? 0.0:1.0);
}