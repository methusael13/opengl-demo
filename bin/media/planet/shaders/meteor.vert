#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in mat4 instanceMatrix;

out VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 tex_coord;
} vs_out;

uniform mat4 pvMat;

void main(void) {
    vec4 _pos = vec4(position, 1.0);
    gl_Position = pvMat * instanceMatrix * _pos;

    vs_out.frag_pos = vec3(instanceMatrix * _pos);
    vs_out.tex_coord = tex_coord;
    vs_out.normal = normal;
}
