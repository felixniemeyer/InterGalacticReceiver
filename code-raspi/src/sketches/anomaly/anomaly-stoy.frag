#version 300 es

precision highp float;

const float rayGravity = 0.25f;
const float terrainHeight = 5.0f;

const float PI = 3.14159265358979323846f;
const float halfPI = 0.5f * PI;
const int MAX_STEPS = 9;

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

vec3 permute(vec3 x) {
  return mod289(((x * 34.0f) + 10.0f) * x);
}

float snoise2d(vec2 v) {
  const vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);
  vec2 i = floor(v + dot(v, C.yy));
  vec2 x0 = v - i + dot(i, C.xx);
  vec2 i1 = (x0.x > x0.y) ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

  i = mod289(i);
  vec3 p = permute(permute(i.y + vec3(0.0f, i1.y, 1.0f)) + i.x + vec3(0.0f, i1.x, 1.0f));

  vec3 m = max(0.5f - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0f);
  m = m * m;
  m = m * m;

  vec3 x = 2.0f * fract(p * C.www) - 1.0f;
  vec3 h = abs(x) - 0.5f;
  vec3 ox = floor(x + 0.5f);
  vec3 a0 = x - ox;

  m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);

  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0f * dot(m, g);
}

// Cheaper 2D simplex – approximate permute + weaker mod
vec3 mod289v(vec3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}
vec3 permuteCheap(vec3 x) {
  return mod289v((34.0f * x + 10.0f) * x);   // keep one mod, or fract it
    // or ultra-cheap: return fract(x * x * 34.0 + x);
}

float snoise2d_cheap(vec2 v) {
  const vec4 C = vec4(0.211324865405187f, 0.366025403784439f, -0.577350269189626f, 0.024390243902439f);
  vec2 i = floor(v + dot(v, C.yy));
  vec2 x0 = v - i + dot(i, C.xx);

  vec2 i1 = (x0.x > x0.y) ? vec2(1.0f, 0.0f) : vec2(0.0f, 1.0f);

  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

    // Cheaper permute chain
    // vec3 pi = mod289v(vec3(i, 1.0));   // weaker or remove
  vec3 p = permuteCheap(permuteCheap(vec3(i.y, i.y + i1.y, i.y + 1.0f)) + vec3(i.x, i.x + i1.x, i.x + 1.0f));

  vec3 m = max(0.5f - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0f);
  m = m * m;
  m = m * m;

  vec3 x = fract(p * C.www) * 2.0f - 1.0f;
  vec3 h = abs(x) - 0.5f;
  vec3 ox = floor(x + 0.5f);
  vec3 a0 = x - ox;

  m *= 1.79284291400159f - 0.85373472095314f * (a0 * a0 + h * h);

  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0f * dot(m, g);
}

float hash2d_blocky(vec2 p, float freq) {
  p *= freq;                      // frequency / scale
  p = floor(p);                   // ← this is the key: snap to integer grid
  p *= vec2(0.3183099f, 0.3678794f);
  p += p.yx * 17.0f + p.xy;
  return fract(p.x * p.y * (p.x + p.y));
}

float surface(vec2 p) {
  float noise1 = snoise2d_cheap(p);
  p.x += noise1;
  float noise2 = snoise2d_cheap(p * 2.f); // second octave
  return (noise1 + noise2 * 0.5f) * 0.33f + 0.5f;
}

float fbmAdaptiveLoop(vec2 p, float distance) {
  float value = 0.0f;
  float amplitude = 0.5f;
  float frequency = 1.0f;
  float noise = 0.f;

  vec2 offset = vec2(0, 0);
  for(int i = 0; i < 2; i++) {
    if(amplitude * terrainHeight < 0.1f * distance)
      break;
    // value += amplitude * (snoise2d(p * frequency) * 0.5 + 0.5);
    // value += amplitude * (hash2d_blocky(p, frequency));
    noise = snoise2d_cheap((p + vec2(noise, 0)) * frequency);
    value += amplitude * (noise * 0.5f + 0.5f);
    frequency *= 2.0f;
    amplitude *= 0.5f;
  }

  return value;
}

float sdf(vec3 p) {
  float height = -surface(p.xz * 0.3f) * terrainHeight; 

  // height = (1. + boost * 3.) * height + terrainHeight * boost; 

  return p.y - height;
}

float cheapHash(vec2 p, float iTime) {
  p = p * 1.0f + iTime * 0.001f;
  return fract(sin(dot(p, vec2(127.1f, 311.7f))) * 43758.5453f);
}

float artifactHash(vec2 p, float iTime) {
  p = p * 100.f + iTime * 0.005f;                    // animate cheaply
  return abs(fract(p.x * p.y + p.x + p.y) * 2.f - 1.f);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 uv = fragCoord / iResolution.xy;
  const float aspectRatio = 720.0f / 576.0f;
  const float invCornerRadius = 1.0f / 1.60078125f;

  vec2 screenUV = uv * 2.0f - 1.0f;
  screenUV.x *= aspectRatio;
  vec2 xz = screenUV * invCornerRadius;

  float uvRnd = cheapHash(xz, iTime);

  float radiusSquared = dot(xz, xz);
  float y = -sqrt(1.0f - radiusSquared);
  vec3 eyeDirection = vec3(xz.x, y, xz.y);

  vec3 cameraPos = vec3(iTime, 2, 0); 
  // rotation 
  float rt = sin(iTime * 0.3f);
  vec2 up = vec2(sin(rt), cos(rt));
  vec2 right = vec2(up.y, -up.x);
  eyeDirection.xz = vec2(up.x * eyeDirection.z + right.x * eyeDirection.x, up.y * eyeDirection.z + right.y * eyeDirection.x);

  float skipDistance = cameraPos.y;
  vec3 pos = cameraPos + eyeDirection * skipDistance;
  float depth = skipDistance;

  vec3 rayDir = normalize(eyeDirection + vec3(0, rayGravity * skipDistance, 0));

  float stepSize = 0.7f + 0.2f * uvRnd;

  for(int i = 0; i < MAX_STEPS; i++) {
    float dist = sdf(pos);
    float step = dist * stepSize;
    depth += step;
    pos += rayDir * step;

    rayDir.y += rayGravity * step * 0.5f;
    if(depth > 50.f)
      break;
  }

  depth = max(0.f, depth);

  float heightFactor = clamp((pos.y + terrainHeight) / terrainHeight, 0.0f, 1.0f);

  float hfsquare = heightFactor * heightFactor;
  float invhf = 1.f - heightFactor;
  float invhfsquare = invhf * invhf;

  vec3 terrainColor = vec3(heightFactor, 0, invhfsquare * 1.3f);

  terrainColor *= 1.f - invhfsquare;
  terrainColor = mix(terrainColor, vec3(1), hfsquare * hfsquare);
  terrainColor *= abs(fract(heightFactor * 70.f) - 0.5f) + 0.7f;

  vec3 skyColor = vec3(-0.2f, -0.1f, 0);

  float fog = 1.f - 5.f / (depth - terrainHeight * 0.5f + 5.0f);
  vec3 color = clamp(mix(terrainColor * 2.f, skyColor, fog), 0.f, 1.f);

  fragColor = vec4(color, 1.0f);
}