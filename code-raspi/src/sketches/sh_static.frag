#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;

out vec4 fragColor;

float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 78.233);
    return fract(sin(p.x + p.y) * 43758.5453);
}

void main() {
    fragColor.a = 1.0;
    vec2 uv = gl_FragCoord.xy / resolution;
    float n = hash(floor(uv * resolution.x / 2.0) + time);
    vec3 nz = vec3(step(0.85, n));
    fragColor.rgb = nz * 0.5;
}
