#version 330 core

in VS_OUT {
    vec2 tex_coord;
    vec3 normal;
    vec3 refract_ray;
} fs_in;

out vec4 color;

uniform float ed_coeff = 0.6;  // Environment diffuse coefficient

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;

uniform samplerCube cube_map;

void main(void) {
    vec3 env_col  = vec3(texture(cube_map, fs_in.refract_ray));
    color = vec4(env_col, 1.0);
}
