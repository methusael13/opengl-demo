#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

out VS_OUT {
    vec2 tex_coord;
    vec3 normal;
    vec3 refract_ray;
} vs_out;

uniform vec3 cam_pos;

uniform mat3 normal_mat;
uniform mat4 model_mat;
uniform mat4 transform;

void main(void) {
    vec4 pos = vec4(position, 1.0);
    vec3 eye_dir = normalize(vec3(model_mat * pos) - cam_pos);

    vs_out.normal = normalize(normal_mat * normal);
    vs_out.refract_ray = normalize(refract(eye_dir, vs_out.normal, 1.0 / 1.0));
    vs_out.tex_coord = tex_coord;

    gl_Position = transform * pos;
}
