#version 330 core

in VS_OUT {
    float tex_mix;
    vec2 tex_coord;
} fs_in;

out vec4 color;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main(void) {
    color = mix(texture(texture0, fs_in.tex_coord), 
                texture(texture1, fs_in.tex_coord),
                fs_in.tex_mix);
}
