#version 310 es
precision highp float;

uniform float time;
uniform vec2 resolution;
out vec4 fragColor;

const float rayGravity = 0.25;
const float terrainHeight = 5.0;

const float PI = 3.14159265358979323846;
const float halfPI = 0.5 * PI;

const int MAX_STEPS = 7;
float hash21(vec2 p) {
  p = fract(p * vec2(123.34, 456.21));
  p += dot(p, p + 23.45);
  return fract(p.x * p.y);
}

float noise2d(vec2 p) {
  vec2 i = floor(p);
  vec2 f = fract(p);
  vec2 u = f * f * (3.0 - 2.0 * f);
  float a = hash21(i);
  float b = hash21(i + vec2(1.0, 0.0));
  float c = hash21(i + vec2(0.0, 1.0));
  float d = hash21(i + vec2(1.0, 1.0));
  return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float surface(vec2 p) {
  float n1 = noise2d(p);
  float n1x2 = n1 + n1; 
  p.x += n1x2;
  float n2 = noise2d(p + p);
  return (n1x2 + n2) * 0.333333;
}


float sdf(vec3 p) {
  float height = -surface(10. + p.xz * 0.7) * terrainHeight; 
  p.y -= abs(fract(time * 0.1 + p.x * 0.1) - 0.5); 
  return p.y - height;
}

float cheapHash(vec2 p, float time) {
  return hash21(p + time * 0.001);
}

const float aspectRatio = 720.0 / 576.0;
const float invCornerRadius = 1.0 / 1.60078125;

void main() {
  vec2 uv = gl_FragCoord.xy / resolution;
  vec2 screenUV = uv * 2.0 - 1.0;
  screenUV.x *= aspectRatio;
  vec2 xz = screenUV * invCornerRadius;

  float uvRnd = cheapHash(xz, time); 

  float radiusSquared = dot(xz, xz);
  float y = -sqrt(max(1.0 - radiusSquared, 0.0));
  vec3 eyeDirection = vec3(xz.x, y, xz.y);

  vec3 cameraPos = vec3(time,2,0); 
  float rt = sin(time * 0.3); 
  vec2 up = vec2(
    sin(rt), 
    cos(rt)
  ); 
  vec2 right = vec2(
    up.y, 
    -up.x
  ); 
  eyeDirection.xz = vec2(
    up.x * eyeDirection.z + right.x * eyeDirection.x, 
    up.y * eyeDirection.z + right.y * eyeDirection.x
  ); 

  float skipDistance = cameraPos.y;
  vec3 pos = cameraPos + eyeDirection * skipDistance;
  float depth = skipDistance;

  vec3 rayDir = eyeDirection + vec3(0, rayGravity * skipDistance, 0);
  rayDir *= inversesqrt(dot(rayDir, rayDir));

  float stepSize = 0.7 + 0.3 * uvRnd; 

  for (int i = 0; i < MAX_STEPS; i++) {
    float dist = sdf(pos);
    float step = dist * stepSize; 
    depth += step;
    pos += rayDir * step;

    rayDir.y += rayGravity * step * 0.5;
  }

  depth = max(0., depth); 

  float invTerrainHeight = 1.0 / terrainHeight;
  float heightFactor = clamp((pos.y + terrainHeight) * invTerrainHeight, 0.0, 1.0);

  float hfsquare = heightFactor * heightFactor; 
  float invhf = 1. - heightFactor; 
  float invhfsquare = invhf * invhf; 

  vec3 terrainColor = vec3(
    heightFactor, 
    0, 
    invhfsquare * 1.3
  ); 

  terrainColor *= 1. - invhfsquare; 
  terrainColor = mix(terrainColor, vec3(1), hfsquare * hfsquare); 
  terrainColor *= abs(fract(heightFactor * 70.) - 0.5) + 0.7; 

  vec3 skyColor = vec3(-0.2,-0.1,0);

  float fog = 1. - 5. / (depth - terrainHeight * 0.5 + 5.0);
  vec3 color = clamp(mix(terrainColor * 2., skyColor, fog), 0., 1.); 

  fragColor = vec4(color, 1.0);
}
