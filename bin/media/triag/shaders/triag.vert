#version 330 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 color;

out VS_OUT {
    vec4 color;
} vs_out;

void main(void) {
    gl_Position = vPosition;

    // Out data
    vs_out.color = color;
}
