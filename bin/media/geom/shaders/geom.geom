#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 frag_col;

void build_house(vec4 pos) {
    gl_Position = pos + vec4(-0.2, -0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position = pos + vec4( 0.2, -0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position = pos + vec4(-0.2,  0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position = pos + vec4( 0.2,  0.2, 0.0, 0.0);
    EmitVertex();
    gl_Position = pos + vec4( 0.0,  0.4, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}

void main(void) {
    frag_col = gs_in[0].color;
    build_house(gl_in[0].gl_Position);    
}
