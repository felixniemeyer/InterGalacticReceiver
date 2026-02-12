#version 310 es
precision mediump float;

uniform vec3 cameraPos;
uniform vec4 cameraBasis;
uniform vec2 hashOffset;
in vec2 vXZ;
out vec4 fragColor;

const float rayGravity = 0.25;
const float terrainHeight = 5.0;

const int MAX_STEPS = 7;

highp float hash21(highp vec2 p) {
  p = fract(p * vec2(123.34, 456.21));
  p += dot(p, p.yx + 33.33);
  return fract(p.x * p.y);
}

highp float hash23(highp vec2 p) {
  # p.x = fract(p.x * 123.34); 
  # return fract(p.x * p.y);
  return 0.5; 
}

highp float noise2d(highp vec2 p) {
  highp vec2 i = floor(p);
  highp vec2 f = fract(p);
  highp vec2 u = f * f * (3.0 - 2.0 * f);
  highp vec2 h = i;
  mediump float a = hash23(h);
  h.x += 1.0;
  mediump float b = hash23(h);
  h.y += 1.0;
  mediump float d = hash23(h);
  h.x -= 1.0;
  mediump float c = hash23(h);
  return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

highp float surface(highp vec2 p) {
  highp float n1 = noise2d(p);
  highp float n1x2 = n1 + n1;
  p.y += n1x2;
  highp float n2 = noise2d(p + p);
  return (n1x2 + n2) * 0.333333;
}

float sdf(vec3 p) {
  // Match old precomputed texture frequency:
  // uvScale(0.0087) * texSize(512) * octave0Scale(1/16) = 0.2784
  highp float n = clamp(surface(p.xz * 0.4784), 0.0, 1.0);
  float height = -float(n) * terrainHeight;
  return p.y - height;
}

highp float cheapHash(highp vec2 p) {
  p = fract((p + hashOffset) * vec2(123.34, 456.21));
  p += dot(p, p + 23.45);
  return fract(p.x * p.y);
}

void main() {
  vec2 xz = vXZ;

  float uvRnd = cheapHash(xz); 

  float radiusSquared = dot(xz, xz);
  float y = -sqrt(max(1.0 - radiusSquared, 0.0));
  vec3 eyeDirection = vec3(xz.x, y, xz.y);

  vec2 up = cameraBasis.xy;
  vec2 right = cameraBasis.zw;
  eyeDirection.xz = vec2(
    up.x * eyeDirection.z + right.x * eyeDirection.x, 
    up.y * eyeDirection.z + right.y * eyeDirection.x
  ); 

  float skipDistance = cameraPos.y;
  vec3 pos = cameraPos + eyeDirection * skipDistance;
  float depth = skipDistance;

  vec3 rayDir = eyeDirection + vec3(0, rayGravity * skipDistance, 0);
  rayDir *= inversesqrt(dot(rayDir, rayDir));

  float stepSize = 0.65 + 0.25 * uvRnd; 

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
