#version 330 core

in VS_OUT {
    vec3 reflect_ray;
} fs_in;

out vec4 color;

uniform samplerCube cube_map;

void main(void) {
    color = texture(cube_map, fs_in.reflect_ray);
}
