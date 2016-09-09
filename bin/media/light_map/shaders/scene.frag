#version 330 core

in VS_OUT {
    vec3 normal;
    vec3 frag_pos;
    vec2 tex_coord;
} fs_in;

uniform sampler2D material_diff_map;
uniform sampler2D material_spec_map;

uniform struct {
    vec3 ambience;      // World ambient color
} world;

uniform struct {
    float ambience;      // Amount of ambience the material receives
    float shininess;     // [1.0, 500.0]
    float specular_int;  // Specular intensity [0.0, 1.0]
} material;

uniform struct {
    vec3  color;
    vec3  position;
    float intensity;
    float linear;
    float quadratic;
} light;

uniform vec3 cam_pos;

out vec4 color;

// Implements phong shading
void main(void) {
    vec3 diff_map = vec3(texture(material_diff_map, fs_in.tex_coord));

    // Calculate ambient color
    vec3 ambient = world.ambience * material.ambience * diff_map;

    // Calculate diffuse color
    vec3 light_dir = normalize(light.position - fs_in.frag_pos);
    float diff_coeff = max(dot(fs_in.normal, light_dir), 0.0);
    vec3 diffuse = light.color * diff_coeff * light.intensity * diff_map;

    // Specular
    float spec_coeff = 0.0;
    if (diff_coeff > 0.0) {
        vec3 cam_dir = normalize(cam_pos - fs_in.frag_pos);
        spec_coeff = pow(max(dot(cam_dir, reflect(
            -light_dir, fs_in.normal)), 0.0), material.shininess);
    }
    vec3 specular = light.color * material.specular_int *
        vec3(texture(material_spec_map, fs_in.tex_coord)) * spec_coeff; 

    // Attenuation
    float light_dist = length(light.position - fs_in.frag_pos);
    float attenuation = 1.0 /
        (1.0 + light_dist * (light.linear + light_dist * light.quadratic));

    color = vec4(ambient + attenuation * (diffuse + specular), 1.0);
}
