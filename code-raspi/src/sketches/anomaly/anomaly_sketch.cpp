#include "anomaly_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global
#include <cmath>

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

namespace {
static const int NOISE_TEX_SIZE = 512;
static const float NOISE_PERIOD_UNITS = 32.0f;
static const float OCTAVE0_SCALE = 1.0f / 16.0f;
static const float OCTAVE1_SCALE = 1.0f / 8.0f;

static const char *noise_gen_vert = R"(
#version 310 es
precision highp float;
layout(location = 0) in vec2 position;
out vec2 uv;
void main() {
  uv = position * 0.5 + 0.5;
  gl_Position = vec4(position, 0.0, 1.0);
}
)";

static const char *noise_gen_frag = R"(
#version 310 es
precision highp float;
in vec2 uv;
out vec4 fragColor;

uniform float octave0Scale;
uniform float octave1Scale;
uniform float noiseTexSize;

vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x * 34.0) + 1.0) * x); }

float simplex2d(vec2 v)
{
  const vec4 C = vec4(
      0.211324865405187,
      0.366025403784439,
     -0.577350269189626,
      0.024390243902439);

  vec2 i = floor(v + dot(v, C.yy));
  vec2 x0 = v - i + dot(i, C.xx);

  vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

  i = mod289(i);
  vec3 p = permute(
      permute(i.y + vec3(0.0, i1.y, 1.0)) +
              i.x + vec3(0.0, i1.x, 1.0));

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
  g.y = a0.y * x12.x + h.y * x12.y;
  g.z = a0.z * x12.z + h.z * x12.w;
  return 130.0 * dot(m, g);
}

float periodicSimplex2(vec2 p, vec2 period)
{
  vec2 q = p / period;
  vec2 f = fract(q);

  float n00 = simplex2d(p);
  float n10 = simplex2d(p - vec2(period.x, 0.0));
  float n01 = simplex2d(p - vec2(0.0, period.y));
  float n11 = simplex2d(p - period);

  float nx0 = mix(n00, n10, f.x);
  float nx1 = mix(n01, n11, f.x);
  return mix(nx0, nx1, f.y);
}

void main() {
  // Work in texel space so octave scales are intuitive (e.g. 1/16, 1/8).
  vec2 texelP = uv * noiseTexSize;
  vec2 p0 = texelP * octave0Scale;
  vec2 period0 = vec2(noiseTexSize * octave0Scale);
  float o0 = periodicSimplex2(p0, period0) * 0.5 + 0.5;

  vec2 p1 = texelP * octave1Scale;
  vec2 period1 = vec2(noiseTexSize * octave1Scale);
  float o0x2 = o0 + o0;
  p1.x += o0x2;
  float o1 = periodicSimplex2(p1, period1) * 0.5 + 0.5;

  float mixed = clamp((o0x2 + o1) * (1.0 / 3.0), 0.0, 1.0);
  fragColor = vec4(vec3(mixed), 1.0);
}
)";

static GLuint create_noise_texture_gpu(const std::vector<GLfloat> &quad)
{
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, NOISE_TEX_SIZE, NOISE_TEX_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  GLuint fbo = 0;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  GLuint vs = SketchBase::compile_shader(GL_VERTEX_SHADER, noise_gen_vert);
  GLuint fs = SketchBase::compile_shader(GL_FRAGMENT_SHADER, noise_gen_frag);
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glBindAttribLocation(prog, 0, "position");
  glLinkProgram(prog);
  GLint ok = 0;
  glGetProgramiv(prog, GL_LINK_STATUS, &ok);
  if (!ok) SketchBase::throw_shader_link_error(prog);

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quad.size(), &quad[0], GL_STATIC_DRAW);

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, NOISE_TEX_SIZE, NOISE_TEX_SIZE);
  glUseProgram(prog);
  GLint scale0_loc = glGetUniformLocation(prog, "octave0Scale");
  GLint scale1_loc = glGetUniformLocation(prog, "octave1Scale");
  GLint texsz_loc = glGetUniformLocation(prog, "noiseTexSize");
  glUniform1f(scale0_loc, OCTAVE0_SCALE);
  glUniform1f(scale1_loc, OCTAVE1_SCALE);
  glUniform1f(texsz_loc, (float)NOISE_TEX_SIZE);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDeleteBuffers(1, &vbo);
  glDeleteProgram(prog);
  glDeleteShader(fs);
  glDeleteShader(vs);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  return tex;
}
} // namespace

AnomalySketch::AnomalySketch(int w, int h, GLuint render_fbo)
    : FragSketch(w, h, render_fbo, anomaly_frag)
{
}

void AnomalySketch::init()
{
    vs = compile_shader(GL_VERTEX_SHADER, anomaly_vert);
    fs = compile_shader(GL_FRAGMENT_SHADER, anomaly_frag);

    prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    const GLint ixPosAttribute = 0;
    glBindAttribLocation(prog, ixPosAttribute, "position");
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) throw_shader_link_error(prog);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glGenBuffers(1, &anomaly_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, anomaly_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quad.size(), &quad[0], GL_STATIC_DRAW);

    glUseProgram(prog);
    camera_pos_loc = glGetUniformLocation(prog, "cameraPos");
    camera_basis_loc = glGetUniformLocation(prog, "cameraBasis");
    hash_offset_loc = glGetUniformLocation(prog, "hashOffset");
    noise_tex_loc = glGetUniformLocation(prog, "noiseTex");
    noise_period_loc = glGetUniformLocation(prog, "noisePeriod");

    noise_tex = create_noise_texture_gpu(quad);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noise_tex);
    glUniform1i(noise_tex_loc, 0);
    glUniform1f(noise_period_loc, NOISE_PERIOD_UNITS);
}

void AnomalySketch::frame(double dt)
{
    time += dt;

    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noise_tex);

    glBindBuffer(GL_ARRAY_BUFFER, anomaly_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    float t = (float)time;
    float rt = sinf(t * 0.3f);
    float upx = sinf(rt);
    float upy = cosf(rt);
    float rightx = upy;
    float righty = -upx;

    glUniform3f(camera_pos_loc, t, 2.0f, 0.0f);
    glUniform4f(camera_basis_loc, upx, upy, rightx, righty);
    float h0 = t * 0.001f;
    float h1 = t * 0.001731f + 0.37f;
    glUniform2f(hash_offset_loc, h0, h1);

    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, w, h);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AnomalySketch::unload(double current_time)
{
    if (anomaly_vbo != 0)
    {
        glDeleteBuffers(1, &anomaly_vbo);
        anomaly_vbo = 0;
    }
    if (noise_tex != 0)
    {
        glDeleteTextures(1, &noise_tex);
        noise_tex = 0;
    }
    glDeleteProgram(prog);
    prog = 0;
    glDeleteShader(fs);
    fs = 0;
    glDeleteShader(vs);
    vs = 0;
}

void AnomalySketch::reload(double current_time)
{
    time = current_time;
    init();
}
