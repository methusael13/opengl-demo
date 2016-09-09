#version 330 core

in  vec3 frag_col;
out vec4 color;

void main(void) {
    color = vec4(frag_col, 1.0);
}
