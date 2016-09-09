#version 330 core

layout (location = 0) in vec3 position;

out vec3 tex_coord;

uniform mat4 transform;

void main(void) {
    vec4 pos = transform * vec4(position, 1.0);
    gl_Position = pos.xyww;

    tex_coord = position;
}
