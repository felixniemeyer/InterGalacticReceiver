#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

void main() {
    vec3 clr = vec3(0.6, 0.0, 0.0);
    fragColor = vec4(1.0);
}
