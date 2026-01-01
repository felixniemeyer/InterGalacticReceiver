#ifndef RAY_SHADERS_H
#define RAY_SHADERS_H

constexpr const char *ray_frag = R"(
#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

void main() {
    vec3 clr = vec3(0.6, 0.0, 0.0);
    fragColor = vec4(1.0);
}
)";

#endif
