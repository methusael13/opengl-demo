#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in float tex_mix;

out VS_OUT {
    float tex_mix;
    vec2 tex_coord;
} vs_out;

void main(void) {
    gl_Position = vec4(position, 1.0) + vec4(offset, 0.0);

    vs_out.tex_mix = tex_mix;
    // Invert incoming image coords on y axis
    vs_out.tex_coord = vec2(tex_coord.x, 1 - tex_coord.y);
}
