#version 330 core

// @author: Methusael Murmu

in VS_OUT {
    vec3 frag_pos;
    vec3 normal;
    vec2 tex_coord;
} fs_in;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Light {
    vec3    direction;      // Only used for directional lights (Sun)
    vec3    diffuse;        // a.k.a Light color
    float   intensity;
};

uniform Light light;
out vec4 color;

// Function prototypes
vec3 calc_dir_light(Light dlight, vec3 diff_map);

// Implements phong shading
void main(void) {
    vec3 out_col = vec3(0);
    vec3 diff_map = vec3(texture(texture_diffuse0, fs_in.tex_coord));

    // Calculate color from lights
    out_col += calc_dir_light(light, diff_map);
    color = vec4(out_col, 1.0);
}

vec3 calc_dir_light(Light dlight, vec3 diff_map) {
    // Diffuse
    vec3  light_dir   = normalize(-dlight.direction);
    float diff_coeff  = max(dot(fs_in.normal, light_dir), 0.0);
    vec3  diffuse     = dlight.diffuse * diff_coeff * dlight.intensity * diff_map;
    
    return diffuse;
}
