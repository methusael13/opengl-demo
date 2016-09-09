#version 330 core

// Contains both coords and tex-coords
layout (location = 0) in vec4 vert;

out vec2 texCoords;

uniform mat4 pMat;

void main(void) {
    gl_Position = pMat * vec4(vert.xy, 0.0, 1.0);
    texCoords = vert.zw;
}
