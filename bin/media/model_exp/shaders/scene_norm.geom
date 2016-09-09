#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 _normal[];

const float norm_length = 0.02;

void display_normal(int idx) {
    gl_Position = gl_in[idx].gl_Position;
    EmitVertex();
    gl_Position = gl_in[idx].gl_Position + vec4(_normal[idx], 0.0) *
                  norm_length;
    EmitVertex();
    EndPrimitive();
}

void main(void) {
    display_normal(0);
    display_normal(1);
    display_normal(2);
}
