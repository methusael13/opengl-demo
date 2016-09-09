#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 normal;
    vec3 frag_pos;
} vs_out;

uniform mat4 model;
uniform mat4 transform;
uniform mat3 normal_mat;

void main(void) {
    vec4 _pos = vec4(position, 1.0);
    gl_Position = transform * _pos;

    vs_out.frag_pos = vec3(model * _pos);
    vs_out.normal = normalize(normal_mat * normal);
}
