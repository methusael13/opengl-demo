#version 330 core

// @author: Methusael Murmu

in VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} gs_in;

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

out vec4 color;

// Function prototypes
vec3 calc_point_light(Light plight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map);
vec3 calc_dir_light(Light dlight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map);
vec3 calc_spot_light(Light slight, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map);

float calc_attenuation(float linear, float quad, float dist);

// Implements phong shading
void main(void) {
    vec3 out_col = vec3(0);
    vec3 cam_dir = normalize(cam_pos - gs_in.frag_pos);
    vec3 diff_map = vec3(texture(texture_diffuse0, gs_in.tex_coord));
    vec3 spec_map = vec3(texture(texture_specular0, gs_in.tex_coord));

    // Calculate color from lights
    out_col += calc_dir_light(light[0], cam_dir, diff_map, spec_map);
    out_col += calc_dir_light(light[1], cam_dir, diff_map, spec_map);
    out_col += calc_dir_light(light[2], cam_dir, diff_map, spec_map);

    // Calculate ambient color
    vec3 ambient = world.ambience * material.ambience * diff_map;
    color = vec4(ambient + out_col, 1.0);
}

float calc_attenuation(float linear, float quad, float dist) {
    float attenuation = 1.0 / (1.0 + linear * dist + quad * dist * dist);
    return attenuation;
}

vec3 calc_dir_light(Light dlight, vec3 cam_dir, vec3 diff_map,
                    vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(-dlight.direction);
    float diff_coeff = max(dot(gs_in.normal, light_dir), 0.0);
    vec3 diffuse = dlight.diffuse * diff_coeff * dlight.intensity * diff_map;

    // Specular
    float spec_coeff = 0.0;
    if (diff_coeff > 0.0) {
        spec_coeff = pow(max(dot(cam_dir, reflect(
            -light_dir, gs_in.normal)), 0.0), material.shininess);
    }
    vec3 specular = dlight.diffuse *
        material.specular_int * spec_map * spec_coeff;

    return (diffuse + specular);
}

vec3 calc_spot_light(Light slight, vec3 cam_dir, vec3 diff_map,
                     vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(slight.position - gs_in.frag_pos);

    float theta = dot(light_dir, normalize(-slight.direction));
    float intensity = clamp(
        (theta - slight.cutoff_out) / slight.epsilon, 0.0, 1.0);

    float diff_coeff = intensity * max(dot(gs_in.normal, light_dir), 0.0);
    vec3 diffuse = slight.diffuse * slight.intensity * diff_coeff * diff_map;

    // Specular
    float spec_coeff = 0.0;
    if (diff_coeff > 0.0) {
        diff_coeff = pow(max(dot(cam_dir, reflect(
            -light_dir, gs_in.normal)), 0.0), material.shininess);
    }
    vec3 specular = slight.diffuse * intensity *
        material.specular_int * spec_coeff * spec_map;

    // Attenuation
    float attenuation = calc_attenuation(slight.linear,
        slight.quadratic, length(slight.position - gs_in.frag_pos));

    return attenuation * (diffuse + specular);
}

vec3 calc_point_light(Light plight, vec3 cam_dir, vec3 diff_map,
                      vec3 spec_map) {
    // Diffuse
    vec3 light_dir = normalize(plight.position - gs_in.frag_pos);
    float diff_coeff = max(dot(gs_in.normal, light_dir), 0.0);
    vec3 diffuse = plight.diffuse * diff_coeff * plight.intensity * diff_map;

    // Specular
    float spec_coeff = 0.0;
    if (diff_coeff > 0.0) {
        spec_coeff = pow(max(dot(cam_dir, reflect(
            -light_dir, gs_in.normal)), 0.0), material.shininess);
    }
    vec3 specular = plight.diffuse *
        material.specular_int * spec_map * spec_coeff; 

    // Attenuation
    float attenuation = calc_attenuation(plight.linear,
        plight.quadratic, length(plight.position - gs_in.frag_pos));

    return attenuation * (diffuse + specular);
}
