#ifndef SHADERS_H
#define SHADERS_H

constexpr const char *vert_glsl = R"(
#version 310 es
precision highp float;
layout(location = 0) in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

constexpr const char *frag_glsl = R"(
#version 310 es
precision highp float;
uniform float time;
uniform vec2 resolution;
out vec4 fragColor;
vec2 uvN(){ return gl_FragCoord.xy / resolution; }
vec2 uv(){ return (gl_FragCoord.xy / resolution * 2.0 - 1.0) * vec2(resolution.x/resolution.y, 1.0); }
#define rot(a) mat2(-1.+2.*noise(a), -sin(a), sin(a), -1.+2.*noise(a))
#define f(z,a) (manualInverse(z)*.15 - a*z)
float rand(const in float n){return fract(sin(n) * 1e4);}
float rand(const in vec2 n) { return fract(1e4 * sin(17.0 * n.x + n.y * 0.1) * (0.1 + abs(sin(n.y * 13.0 + n.x))));
}
float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(rand(i), rand(i + 1.0), u);
}
vec4 star(vec2 u) {
    float v = 8. * atan(u.x, u.y);
    v += 4. * abs(sin(.5 * v + .1 * time));
    v -= 1. * abs(sin(1.5 * v));
    return vec4(.5+.5*cos(v + vec3(2.3+length(u)*0.29,
                6.2+length(u)*3.9 + .0*time,
                3.21+length(u)*3.83
                )), 1.);
}
void main() {
    vec2 u = uv();
    for (int i=0;i++<3;)
        u=vec2(u.x,-u.y)/dot(u,u)+.4*u*rot(time*.05);
//    u/= length(u);
    u = normalize(u)*log(length(u));
    fragColor = vec4(u.x+u.y)*9.;
    fragColor = star(u);
}
)";

#endif
