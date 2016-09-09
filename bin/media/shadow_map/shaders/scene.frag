#version 330 core

// @author: Methusael Murmu

#define MAX_NUM_LIGHTS 5

in VS_OUT {
    vec3 frag_pos;
    vec2 tex_coord;
    vec4 shadow_coord;

    // Tangent space components
    vec3 ts_cam_pos;
    vec3 ts_frag_pos;
    vec3 ts_light_pos[MAX_NUM_LIGHTS];
    vec3 ts_light_dir[MAX_NUM_LIGHTS];
} fs_in;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;
uniform sampler2DShadow shadow_tex;

uniform struct {
    vec3 ambience;          // World ambient color
} world;

uniform struct {
    float ambience;         // Amount of ambience the material receives
    float shininess;        // [1.0, 500.0]
    float specular_int;     // Specular intensity [0.0, 1.0]
} material;

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

uniform int light_count;
uniform Light light[MAX_NUM_LIGHTS];
uniform vec3 cam_pos;

// Gamma inverse
uniform float gamma_inv = 1.0 / 2.2;

layout (location = 0) out vec4 color;

// Function prototypesc
vec3 calc_point_light(int idx, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map, vec3 norm_map);
vec3 calc_dir_light(int idx, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map, vec3 norm_map);
vec3 calc_spot_light(int idx, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map, vec3 norm_map);

float calc_attenuation(float linear, float quad, float dist);
vec3 calc_specular(Light _light, vec3 light_dir, vec3 cam_dir, vec3 spec_map);

vec3 norm_map;

float test_gt(float a, float b) {
    return max(sign(a - b), 0.0);
}

// PCF (8x Samples)
float calc_shadow() {
    vec4 projCoord = fs_in.shadow_coord;

    vec2 texelSize = 1.0 / textureSize(shadow_tex, 0);
    float visibility = textureProj(shadow_tex, projCoord);

    // Sum all surrounding texels
    projCoord.xy = fs_in.shadow_coord.xy + vec2(-1, 1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(0, 1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(1, 1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(1, 0) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(1, -1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(0, -1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(-1, -1) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    projCoord.xy = fs_in.shadow_coord.xy + vec2(-1, 0) * texelSize;
    visibility += textureProj(shadow_tex, projCoord);

    return 1.0 - visibility / 9.0;
}

// Implements Blinn-Phong shading
void main(void) {
    vec3 out_col = vec3(0);
    vec3 cam_dir = normalize(fs_in.ts_cam_pos - fs_in.ts_frag_pos);
    vec3 diff_map = vec3(texture(texture_diffuse0, fs_in.tex_coord));
    vec3 spec_map = vec3(texture(texture_specular0, fs_in.tex_coord));
    norm_map = vec3(texture(texture_normal0, fs_in.tex_coord));
    norm_map = normalize(norm_map * 2.0 - 1.0);

    // Calculate color from lights
    out_col += calc_dir_light(0, cam_dir, diff_map, spec_map, norm_map);

    // Calculate ambient color
    vec3 ambience = world.ambience * material.ambience * diff_map;
    // Apply shadow
    out_col = mix(out_col + ambience, ambience, calc_shadow());

    // Gamma correction
    out_col = pow(out_col, vec3(gamma_inv));
    color = vec4(out_col, 1.0);
}

// Apply only linear attenuation, to account for gamma correction
float calc_attenuation(float linear, float quad, float dist) {
    float attenuation = 1.0 / (1.0 + linear * dist);
    return attenuation;
}

vec3 calc_specular(Light _light, vec3 light_dir, vec3 cam_dir, vec3 spec_map) {
    float spec_coeff = pow(max(dot(norm_map,
        normalize(light_dir + cam_dir)), 0.0), material.shininess);
    return _light.diffuse * material.specular_int * spec_map * spec_coeff;
}

vec3 calc_dir_light(int idx, vec3 cam_dir, vec3 diff_map,
                    vec3 spec_map, vec3 norm_map) {
    Light dlight = light[idx];
    // Diffuse
    vec3 light_dir = -fs_in.ts_light_dir[idx];
    float diff_coeff = max(dot(norm_map, light_dir), 0.0);
    vec3 diffuse = dlight.diffuse * diff_coeff * dlight.intensity * diff_map;

    // Specular
    vec3 specular = vec3(0.0);
    if (diff_coeff > 0.0)
        specular += calc_specular(dlight, light_dir, cam_dir, spec_map);

    return (diffuse + specular);
}

vec3 calc_spot_light(int idx, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map, vec3 norm_map) {
    Light slight = light[idx];
    // Diffuse
    vec3 light_dir = normalize(fs_in.ts_light_pos[idx] - fs_in.ts_frag_pos);

    float theta = dot(light_dir, normalize(-fs_in.ts_light_dir[idx]));
    float intensity = clamp(
        (theta - slight.cutoff_out) / slight.epsilon, 0.0, 1.0);

    float diff_coeff = intensity * max(dot(norm_map, light_dir), 0.0);
    vec3 diffuse = slight.diffuse * slight.intensity * diff_coeff * diff_map;

    // Specular
    vec3 specular = vec3(0.0);
    if (diff_coeff > 0.0)
        specular += calc_specular(slight, light_dir, cam_dir, spec_map);

    // Attenuation
    float attenuation = calc_attenuation(slight.linear,
        slight.quadratic, length(slight.position - fs_in.frag_pos));

    return attenuation * (diffuse + specular);
}

vec3 calc_point_light(int idx, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map) {
    Light plight = light[idx];
    // Diffuse
    vec3 light_dir = normalize(fs_in.ts_light_pos[idx] - fs_in.ts_frag_pos);
    float diff_coeff = max(dot(norm_map, light_dir), 0.0);
    vec3 diffuse = plight.diffuse * diff_coeff * plight.intensity * diff_map;

    // Specular
    vec3 specular = vec3(0.0);
    specular += test_gt(diff_coeff, 0.0) *
        calc_specular(plight, light_dir, cam_dir, spec_map);

    // Attenuation
    float attenuation = calc_attenuation(plight.linear,
        plight.quadratic, length(plight.position - fs_in.frag_pos));

    return attenuation * (diffuse + specular);
}
