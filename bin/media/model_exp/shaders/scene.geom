#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} vs_in[];

out VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} gs_out;

uniform float time;
uniform float magnitude = 0.5;

const float FREQ = 1;       // Frequency
const float DISCONT = 10;   // Discontinuity factor

vec3 calc_normal(void) {
    vec3 l1 = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
    vec3 l2 = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
    return normalize(cross(l1, l2));
}

void explode(vec4 pos, vec3 exp_pos, vec3 norm) {
    float offset = (sin(FREQ * (time + exp_pos.y)) + 1) * 0.5;
    vec3 dir = norm * pow(offset, DISCONT) * magnitude;
    gl_Position = pos + vec4(dir, 0.0);
    EmitVertex();
}

// Pass input block as it is to fragment shader
void pass_through_block(int i) {
    gs_out.normal = vs_in[i].normal;
    gs_out.frag_pos = vs_in[i].frag_pos;
    gs_out.tex_coord = vs_in[i].tex_coord;
}

void main(void) {
    vec3 normal = calc_normal();

    pass_through_block(0);
    explode(gl_in[0].gl_Position, vs_in[0].frag_pos, normal);
    pass_through_block(1);
    explode(gl_in[1].gl_Position, vs_in[1].frag_pos, normal);
    pass_through_block(2);
    explode(gl_in[2].gl_Position, vs_in[2].frag_pos, normal);

    EndPrimitive();
}
