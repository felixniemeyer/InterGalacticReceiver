#ifndef BASE_SHADERS_H
#define BASE_SHADERS_H

constexpr const char *sweep_vert = R"(
#version 310 es
precision highp float;
layout(location = 0) in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

constexpr const char *render_frag = R"(
#version 310 es
precision highp float;

uniform sampler2D tex;
uniform vec2 resolution;
out vec4 fragColor;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    fragColor = texture(tex, uv);
}
)";

#endif
