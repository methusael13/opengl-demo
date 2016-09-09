#version 330 core

// @author: Methusael Murmu

in VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} fs_in;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

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

#define MAX_NUM_LIGHTS 5

uniform int light_count;
uniform Light light[MAX_NUM_LIGHTS];
uniform vec3 cam_pos;

// Gamma inverse
uniform float gamma_inv = 1.0 / 2.2;

out vec4 color;

// Function prototypesc
vec3 calc_point_light(Light plight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map);
vec3 calc_dir_light(Light dlight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map);
vec3 calc_spot_light(Light slight, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map);

float calc_attenuation(float linear, float quad, float dist);
vec3 calc_specular(Light _light, vec3 light_dir, vec3 cam_dir, vec3 spec_map);

float test_gt(float a, float b) {
    return max(sign(a - b), 0.0);
}

// Implements Blinn-Phong shading
void main(void) {
    vec3 out_col = vec3(0);
    vec3 cam_dir = normalize(cam_pos - fs_in.frag_pos);
    vec3 diff_map = vec3(texture(texture_diffuse0, fs_in.tex_coord));
    vec3 spec_map = vec3(texture(texture_specular0, fs_in.tex_coord));

    // Calculate color from lights
    out_col += calc_point_light(light[0], cam_dir, diff_map, spec_map);
    out_col += calc_point_light(light[1], cam_dir, diff_map, spec_map);
    out_col += calc_point_light(light[2], cam_dir, diff_map, spec_map);
    out_col += calc_point_light(light[3], cam_dir, diff_map, spec_map);
    out_col += calc_point_light(light[4], cam_dir, diff_map, spec_map);
    // out_col += calc_dir_light(light[1], cam_dir, diff_map, spec_map);
    // out_col += calc_dir_light(light[2], cam_dir, diff_map, spec_map);

    // Calculate ambient color
    out_col += world.ambience * material.ambience * diff_map;
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
    float spec_coeff = pow(max(dot(fs_in.normal,
        normalize(light_dir + cam_dir)), 0.0), material.shininess);
    return _light.diffuse * material.specular_int * spec_map * spec_coeff;
}

vec3 calc_dir_light(Light dlight, vec3 cam_dir, vec3 diff_map,
                    vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(-dlight.direction);
    float diff_coeff = max(dot(fs_in.normal, light_dir), 0.0);
    vec3 diffuse = dlight.diffuse * diff_coeff * dlight.intensity * diff_map;

    // Specular
    vec3 specular = vec3(0.0);
    if (diff_coeff > 0.0)
        specular += calc_specular(dlight, light_dir, cam_dir, spec_map);

    return (diffuse + specular);
}

vec3 calc_spot_light(Light slight, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(slight.position - fs_in.frag_pos);

    float theta = dot(light_dir, normalize(-slight.direction));
    float intensity = clamp(
        (theta - slight.cutoff_out) / slight.epsilon, 0.0, 1.0);

    float diff_coeff = intensity * max(dot(fs_in.normal, light_dir), 0.0);
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

vec3 calc_point_light(Light plight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(plight.position - fs_in.frag_pos);
    float diff_coeff = max(dot(fs_in.normal, light_dir), 0.0);
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
