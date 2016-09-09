#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

#define MAX_NUM_LIGHTS 5

out VS_OUT {
    vec3 frag_pos;
    vec2 tex_coord;
    vec4 shadow_coord;

    // Tangent space components
    vec3 ts_cam_pos;
    vec3 ts_frag_pos;
    vec3 ts_light_pos[MAX_NUM_LIGHTS];
    vec3 ts_light_dir[MAX_NUM_LIGHTS];
} vs_out;

struct Light {
    int     type;           // Point, Spot or Sun
    vec3    position;
    vec3    direction;      // Only used for directional lights (Sun)
    vec3    diffuse;        // a.k.a Light color
    float   cutoff_out;     // Cosine of outer cutoff angle for spotlights
    float   epsilon;        // OuterCutoff - Cutoff
    float   intensity;
    float   linear;         // Linear attenuation coefficient
    float   quadratic;      // Quadratic attenuation coefficient
};
uniform Light light[MAX_NUM_LIGHTS];

uniform mat4 model;
uniform mat4 transform;
uniform mat3 normal_mat;
uniform mat4 shadow_mat;

uniform vec3 cam_pos;
uniform int light_count;

void main(void) {
    vec4 _pos = vec4(position, 1.0);
    gl_Position = transform * _pos;
    vs_out.shadow_coord = shadow_mat * _pos;  // Scale and bias applied

    vs_out.frag_pos = vec3(model * _pos);
    vs_out.tex_coord = tex_coord;

    // Tangent space calculations
    vec3 t = normalize(normal_mat * tangent);
    vec3 b = normalize(normal_mat * bitangent);
    vec3 n = normalize(normal_mat * normal);
    mat3 tbn_mat = transpose(mat3(t, b, n));

    vs_out.ts_cam_pos = tbn_mat * cam_pos;
    vs_out.ts_frag_pos = tbn_mat * vs_out.frag_pos;
    for (int i = 0; i < light_count; ++i) {
        vs_out.ts_light_pos[i] = tbn_mat * light[i].position;
        vs_out.ts_light_dir[i] = tbn_mat * light[i].direction;
    }
}
