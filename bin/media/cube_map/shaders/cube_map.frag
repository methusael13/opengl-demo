#version 330 core

in vec3 tex_coord;
out vec4 color;

uniform samplerCube cube_map;

void main(void) {
    color = texture(cube_map, tex_coord);
}
