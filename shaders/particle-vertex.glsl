#version 330 core

layout (location = 0) in vec2 aVertex;
layout (location = 1) in vec2 aPosition;

out vec2 fPos;
out vec3 fBaseColor;
out vec2 fTexPos;

uniform int uCount;
uniform int uTheme;
uniform vec2 uCamera;
uniform float uZoom;
uniform float uRatio;
uniform float uSize;

void main() {
    fPos = aVertex;

    float val = float(gl_InstanceID)/float(uCount);

    if (uTheme == 1) fBaseColor = vec3(val, val, val);
    if (uTheme == -1) fBaseColor = vec3(1.0, 0.0, 0.0);
    if (uTheme == 2) fBaseColor = vec3(1.0, val, val);
    if (uTheme == -2) fBaseColor = vec3(0.0, 0.0, 1.0);

    fTexPos = (aVertex*uSize+aPosition+uCamera)*uZoom/vec2(uRatio, 1.0);

    gl_Position = vec4(fTexPos, 1.0, 1.0);
}