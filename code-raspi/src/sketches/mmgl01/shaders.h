#ifndef MMGL01_SHADERS_H
#define MMGL01_SHADERS_H

constexpr const char *mmgl01_frag = R"(
#version 310 es
precision highp float;

// https://www.shadertoy.com/view/lX3XWl

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

vec2 rotate(vec2 p, float a) {
    mat2 m = mat2(cos(a), -sin(a), sin(a), cos(a));
    return p * m;
}

float line(vec2 p, float t) {
    vec2 p1 = rotate(p, t * 0.2);
    float d1 = length(vec2(sin(p1.x * 4.0) * sin(p1.y * 4.0 + t), p1.y * 0.2));

    float d2 = length(vec2(sin(p1.x * 4.0) * sin(p1.y * p.y + t * 0.2), p1.y * 0.2));
    return d1 - d2;
}

// https://iquilezles.org/articles/palettes/
vec3 palette(float t) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.0, 0.10, 0.20);

    return a + b * cos(6.28318 * (c * t + d));
}

void main() {
    vec2 p = (2.0 * gl_FragCoord.xy - resolution.xy) / resolution.y;
    // vec2 mouse = iMouse.xy / resolution.xy + 0.5;
    vec2 mouse = vec2(0.5);

    // Size based on Mouse.x
    p = mix(p * 0.1, p * 0.8, mouse.x);

	// Circular pattern based on mouse.x
    vec2 pc = vec2(length(p), mix(distance(p.x, p.y), p.x * p.y * 0.4, mouse.x));

    pc = mix(pc, vec2(length(sin(pc)), tan(pc.y / pc.x)), 0.4);

	// Lines based on mouse.y
    vec2 p1 = mix(p, tan(pc * 10.0 + time), mouse.y * 0.04);

    vec2 t = vec2(line(p1 / pc, time), line(pc, time * 2.0));
    float t1 = t.x / t.y;
    t1 = t1 / length(p * 8.0);
    float t2 = step(mod(pc.x, abs(sin(time * 0.8))) + 0.2, t1);
    t1 = smoothstep(0.1 * p1.y, abs(sin(time * 0.04 * p1.y)) + 0.4, t1);

    vec3 col = palette(t1 + 0.4);

    fragColor = vec4(col, 1.0);
}
)";

#endif
