#ifndef ANOMALY_SHADERS_H
#define ANOMALY_SHADERS_H

constexpr const char *anomaly_frag = R"(
#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

const float rayGravity = 0.25;
const float terrainHeight = 5.0;

const float PI = 3.14159265358979323846;
const float halfPI = 0.5 * PI;
const int MAX_STEPS = 9;

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x * 34.0) + 10.0) * x);
}

float snoise2d(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    i = mod289(i);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

// Cheaper 2D simplex – approximate permute + weaker mod
vec3 mod289v(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 permuteCheap(vec3 x) {
    return mod289v((34.0 * x + 10.0) * x);   // keep one mod, or fract it
    // or ultra-cheap: return fract(x * x * 34.0 + x);
}

float snoise2d_cheap(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);

    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Cheaper permute chain
    // vec3 pi = mod289v(vec3(i, 1.0));   // weaker or remove
    vec3 p = permuteCheap(permuteCheap(vec3(i.y, i.y + i1.y, i.y + 1.0)) + vec3(i.x, i.x + i1.x, i.x + 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;

    vec3 x = fract(p * C.www) * 2.0 - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float hash2d_blocky(vec2 p, float freq) {
    p *= freq;                      // frequency / scale
    p = floor(p);                   // ← this is the key: snap to integer grid
    p *= vec2(0.3183099, 0.3678794);
    p += p.yx * 17.0 + p.xy;
    return fract(p.x * p.y * (p.x + p.y));
}

float surface(vec2 p) {
    float noise1 = snoise2d_cheap(p);
    p.x += noise1;
    float noise2 = snoise2d_cheap(p * 2.); // second octave
    return (noise1 + noise2 * 0.5) * 0.33 + 0.5;
}

float fbmAdaptiveLoop(vec2 p, float distance) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    float noise = 0.;

    vec2 offset = vec2(0, 0);
    for(int i = 0; i < 2; i++) {
        if(amplitude * terrainHeight < 0.1 * distance)
            break;
        // value += amplitude * (snoise2d(p * frequency) * 0.5 + 0.5);
        // value += amplitude * (hash2d_blocky(p, frequency));
        noise = snoise2d_cheap((p + vec2(noise, 0)) * frequency);
        value += amplitude * (noise * 0.5 + 0.5);
        frequency *= 2.0;
        amplitude *= 0.5;
    }

    return value;
}

float sdf(vec3 p) {
    float height = -surface(p.xz * 0.3) * terrainHeight; 

    // height = (1. + boost * 3.) * height + terrainHeight * boost; 

    return p.y - height;
}

float cheapHash(vec2 p, float time) {
    p = p * 1.0 + time * 0.001;
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float artifactHash(vec2 p, float time) {
    p = p * 100. + time * 0.005;                    // animate cheaply
    return abs(fract(p.x * p.y + p.x + p.y) * 2. - 1.);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;

    float aspectRatio = resolution.x / resolution.y;
    const float invCornerRadius = 1.0 / 1.60078125;

    vec2 screenUV = uv * 2.0 - 1.0;
    screenUV.x *= aspectRatio;
    vec2 xz = screenUV * invCornerRadius;

    float uvRnd = cheapHash(xz, time);

    float radiusSquared = dot(xz, xz);
    float y = -sqrt(1.0 - radiusSquared);
    vec3 eyeDirection = vec3(xz.x, y, xz.y);

    vec3 cameraPos = vec3(time, 2, 0); 
    // rotation 
    float rt = sin(time * 0.3);
    vec2 up = vec2(sin(rt), cos(rt));
    vec2 right = vec2(up.y, -up.x);
    eyeDirection.xz = vec2(up.x * eyeDirection.z + right.x * eyeDirection.x, up.y * eyeDirection.z + right.y * eyeDirection.x);

    float skipDistance = cameraPos.y;
    vec3 pos = cameraPos + eyeDirection * skipDistance;
    float depth = skipDistance;

    vec3 rayDir = normalize(eyeDirection + vec3(0, rayGravity * skipDistance, 0));

    float stepSize = 0.7 + 0.2 * uvRnd;

    for(int i = 0; i < MAX_STEPS; i++) {
        float dist = sdf(pos);
        float step = dist * stepSize;
        depth += step;
        pos += rayDir * step;

        rayDir.y += rayGravity * step * 0.5;
        if(depth > 50.)
            break;
    }

    depth = max(0., depth);

    float heightFactor = clamp((pos.y + terrainHeight) / terrainHeight, 0.0, 1.0);

    float hfsquare = heightFactor * heightFactor;
    float invhf = 1. - heightFactor;
    float invhfsquare = invhf * invhf;

    vec3 terrainColor = vec3(heightFactor, 0, invhfsquare * 1.3);

    terrainColor *= 1. - invhfsquare;
    terrainColor = mix(terrainColor, vec3(1), hfsquare * hfsquare);
    terrainColor *= abs(fract(heightFactor * 70.) - 0.5) + 0.7;

    vec3 skyColor = vec3(-0.2, -0.1, 0);

    float fog = 1. - 5. / (depth - terrainHeight * 0.5 + 5.0);
    vec3 color = clamp(mix(terrainColor * 2., skyColor, fog), 0., 1.);

    fragColor = vec4(color, 1.0);
}
)";

#endif
