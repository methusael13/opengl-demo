#version 330 core

in vec4 frag_pos;

uniform vec3 lightPos;
uniform float farPlane = 100.0;

void main(void) {
    // Map depth to [0, 1]
    gl_FragDepth = length(frag_pos.xyz - lightPos) / farPlane;
}
