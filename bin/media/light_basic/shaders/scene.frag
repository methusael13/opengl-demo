#version 330 core

out vec4 color;

in VS_OUT {
    vec3 normal;
    vec3 frag_pos;
} fs_in;

uniform struct {
    vec3 ambience;      // World ambient color
} world;

uniform struct {
    vec3 diffuse;
    vec3 specular;
    float ambience;      // Amount of ambience the materiol receives
    float shininess;     // [1.0, 500.0]
    float specular_int;  // Specular intensity [0.0, 1.0]
} material;

uniform struct {
    vec3  color;
    vec3  position;
    float intensity;
    float attenuation;
} light;

uniform vec3 cam_pos;

// Implements phong shading
void main(void) {
    // Calculate ambient color
    vec3 ambient = world.ambience * material.diffuse * material.ambience;

    // Calculate diffuse color
    vec3 light_dir = normalize(light.position - fs_in.frag_pos);
    float diff_coeff = max(dot(fs_in.normal, light_dir), 0.0);
    vec3 diffuse = material.diffuse * light.color * diff_coeff *
                   light.intensity;

    // Specular
    float spec_coeff = 0.0;
    if (diff_coeff > 0.0) {
        vec3 cam_dir = normalize(cam_pos - fs_in.frag_pos);
        spec_coeff = pow(max(dot(cam_dir, reflect(
            -light_dir, fs_in.normal)), 0.0), material.shininess);
    }
    vec3 specular = light.color *
        (material.specular * material.specular_int) * spec_coeff; 

    // Attenuation
    float light_dist = length(light.position - fs_in.frag_pos);
    float attenuation = 1.0 / (1.0 + light.attenuation * pow(light_dist, 2));

    color = vec4(ambient + attenuation * (diffuse + specular), 1.0);
}
