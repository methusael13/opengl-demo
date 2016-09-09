#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec3 color;

out VS_OUT {
    vec3 color;
} vs_out;

void main(void) {
    gl_Position = vec4(position, 1.0) + vec4(offset, 0.0);

    // Out data
    vs_out.color = color;
}
