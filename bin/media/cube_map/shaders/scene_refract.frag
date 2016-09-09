#version 330 core

in VS_OUT {
    vec3 position;
    vec3 normal;
} fs_in;

out vec4 color;

uniform vec3 cam_pos;
uniform samplerCube cube_map;

void main(void) {
    vec3 ir = normalize(fs_in.position - cam_pos);
    // Water refraction
    vec3 rr = refract(ir, fs_in.normal, 1.0 / 1.33);

    color = texture(cube_map, rr);
}
