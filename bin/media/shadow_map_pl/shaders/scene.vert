#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

out VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} vs_out;

uniform mat4 model;
uniform mat4 transform;
uniform mat3 normal_mat;

uniform bool invertNormal = false;

void main(void) {
    vec4 _pos = vec4(position, 1.0);
    gl_Position = transform * _pos;

    float sgn = invertNormal ? -1.0 : 1.0;
    vs_out.frag_pos = vec3(model * _pos);
    vs_out.normal = sgn * normalize(normal_mat * normal);
    vs_out.tex_coord = tex_coord;
}
