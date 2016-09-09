#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out vec4 frag_pos;
uniform mat4 lvpMat[6];

void main(void) {
    int f, v;
    for (f = 0; f < 6; ++f) {
        gl_Layer = f;
        for (v = 0; v < 3; ++v) {
            frag_pos = gl_in[v].gl_Position;
            gl_Position = lvpMat[f] * frag_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
