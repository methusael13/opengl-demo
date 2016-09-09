#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 reflect_ray;
} vs_out;

uniform vec3 cam_pos;

uniform mat3 normal_mat;
uniform mat4 model_mat;
uniform mat4 transform;

void main(void) {
    gl_Position = transform * vec4(position, 1.0);

    vec3 normal = normalize(normal_mat * normal);
    vec3 eye_dir = normalize(vec3(model_mat * vec4(position, 1.0)) - cam_pos);
    vs_out.reflect_ray = normalize(reflect(eye_dir, normal));
}
