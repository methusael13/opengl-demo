#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 _normal;

uniform mat4 transform;
uniform mat3 npMat;

void main(void) {
    vec4 _pos = vec4(position, 1.0);
    gl_Position = transform * _pos;

    _normal = normalize(npMat * normal);
}
