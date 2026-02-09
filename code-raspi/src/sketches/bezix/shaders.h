#ifndef BEZIX_SHADERS_H
#define BEZIX_SHADERS_H

constexpr const char *bezix_frag = R"(
#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

//v2
float n(float i) {
    float t = time + 9.0 * cos(time * 0.1);
    return 3. * sin(t * (1. + 2. * sin(i * .01)) + i);
}
float bezier(float t, float a, float b, float c, float d) {
    float q = 1.0 - t;
    return q * q * q * n(a) +
        3. * q * q * t * n(b) +
        3. * q * t * t * n(c) +
        t * t * t * n(d);
}
float color(vec2 uv, float off) {
    vec2 a = vec2(bezier(uv.x, 1.63, -2.06 + off, 3.43, -4.88), bezier(uv.x, 9.48, -8.22, 7.17, -6.11));
    vec2 b = vec2(bezier(uv.y, 5.19, 2.81, 3.73, -5.18), bezier(uv.y, -1.73, -3.43, 8.71, 9.11));
    return distance(a, b);
}
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec3 c = vec3(color(uv, .0), color(uv, .01), color(uv, .02));
    vec3 col = vec3(1.0 / (0.1 + c * 5.0));
    fragColor = vec4(col, 1.0);
}
)";

#endif
