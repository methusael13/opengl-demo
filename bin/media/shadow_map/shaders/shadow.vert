#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 lmvpMat;

void main(void) {
    gl_Position = lmvpMat * vec4(position, 1.0);
}
