#include "anomaly_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global
#include <cmath>
#include <cstdint>
#include <vector>

#ifndef GL_R8
#define GL_R8 0x8229
#endif
#ifndef GL_RED
#define GL_RED 0x1903
#endif

namespace {
static const int NOISE_TEX_SIZE = 512;
static const int NOISE_BASE_PERIOD = 32;
static const float NOISE_PERIOD_UNITS = (float)NOISE_BASE_PERIOD;

static inline uint32_t hash2i(uint32_t x, uint32_t y, uint32_t seed)
{
    uint32_t h = seed;
    h ^= x * 0x9E3779B9u;
    h = (h << 6) | (h >> 26);
    h ^= y * 0x85EBCA6Bu;
    h *= 0xC2B2AE35u;
    h ^= h >> 16;
    return h;
}

static inline float rand01(uint32_t x, uint32_t y, uint32_t seed)
{
    return (float)(hash2i(x, y, seed) & 0x00FFFFFFu) * (1.0f / 16777215.0f);
}

static inline int imod(int v, int m)
{
    int r = v % m;
    return r < 0 ? r + m : r;
}

static float smooth_value_noise_periodic(float x, float y, int period, uint32_t seed)
{
    int x0 = (int)floorf(x);
    int y0 = (int)floorf(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float fx = x - (float)x0;
    float fy = y - (float)y0;
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    uint32_t px0 = (uint32_t)imod(x0, period);
    uint32_t py0 = (uint32_t)imod(y0, period);
    uint32_t px1 = (uint32_t)imod(x1, period);
    uint32_t py1 = (uint32_t)imod(y1, period);

    float a = rand01(px0, py0, seed);
    float b = rand01(px1, py0, seed);
    float c = rand01(px0, py1, seed);
    float d = rand01(px1, py1, seed);

    float ab = a + (b - a) * ux;
    float cd = c + (d - c) * ux;
    return ab + (cd - ab) * uy;
}

static void fill_tileable_noise_r8(std::vector<uint8_t> &data)
{
    data.resize(NOISE_TEX_SIZE * NOISE_TEX_SIZE);
    for (int y = 0; y < NOISE_TEX_SIZE; ++y)
    {
        float v = (float)y / (float)NOISE_TEX_SIZE;
        float py = v * (float)NOISE_BASE_PERIOD;
        for (int x = 0; x < NOISE_TEX_SIZE; ++x)
        {
            float u = (float)x / (float)NOISE_TEX_SIZE;
            float px = u * (float)NOISE_BASE_PERIOD;

            // Match the previous shader's 2-octave shape:
            // n1 -> x-warp -> n2 at doubled frequency -> weighted sum.
            float n1 = smooth_value_noise_periodic(px, py, NOISE_BASE_PERIOD, 0x13579BDFu);
            float n1x2 = n1 + n1;
            float n2 = smooth_value_noise_periodic((px + n1x2) * 2.0f, py * 2.0f, NOISE_BASE_PERIOD * 2, 0x2468ACE1u);
            float mixed = (n1x2 + n2) * (1.0f / 3.0f);

            if (mixed < 0.0f) mixed = 0.0f;
            if (mixed > 1.0f) mixed = 1.0f;
            data[y * NOISE_TEX_SIZE + x] = (uint8_t)lroundf(mixed * 255.0f);
        }
    }
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

    std::vector<uint8_t> noise_data;
    fill_tileable_noise_r8(noise_data);

    glGenTextures(1, &noise_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noise_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, NOISE_TEX_SIZE, NOISE_TEX_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, noise_data.data());
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
