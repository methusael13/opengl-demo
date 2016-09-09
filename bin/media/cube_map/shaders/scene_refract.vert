#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 position;
    vec3 normal;
} vs_out;

uniform mat3 normal_mat;
uniform mat4 model_mat;
uniform mat4 transform;

void main(void) {
    gl_Position = transform * vec4(position, 1.0);

    vs_out.normal = normalize(normal_mat * normal);
    vs_out.position = vec3(model_mat * vec4(position, 1.0));
}
