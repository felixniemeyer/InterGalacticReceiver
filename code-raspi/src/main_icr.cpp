#include "main.h"

// Local dependencies
#include "error.h"
#include "fps.h"
#include "horrors.h"
#include "magic.h"
#include "sketch_base.h"
#include "sketches/shaders.h"
#include "tuner.h"

// Sketches
#include "sketches/star/star_sketch.h"

// Global
#include <vector>

static Tuner tuner;
static std::vector<SketchBase *> sketches;
static int sketch_ix = -1;
static GLuint render_tex = 0;
static GLuint render_depth = 0;
static GLuint render_fbo = 0;
static GLuint render_prog = 0;
static GLuint render_vbo = 0;

static void init_render_target();
static void compile_render_prog();
static void init_stations();
static void render();

void main_icr()
{
    init_render_target();
    compile_render_prog();
    init_stations();
    FPS fps(TARGET_FPS);
    double last_time = fps.frame_start();
    while (app_running)
    {
        double current_time = fps.frame_start();
        double dt = current_time - last_time;
        sketches[sketch_ix]->frame(dt);
        render();
        put_on_screen();
        fps.frame_end();
    }
}

void init_stations()
{
    auto star_sketch = new StarSketch(W, H, render_fbo);
    star_sketch->init();
    sketches.push_back(star_sketch);
    sketch_ix = 0;
}

void init_render_target()
{
    // Texture
    glGenTextures(1, &render_tex);
    glBindTexture(GL_TEXTURE_2D, render_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Framebuffer and attach texture
    glGenFramebuffers(1, &render_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);

    // Depth buffer
    glGenRenderbuffers(1, &render_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, render_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_depth);

    GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (res != GL_FRAMEBUFFER_COMPLETE)
        THROWF("Render target FBO failed to build: 0x%04X", (int)res);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void compile_render_prog()
{
    // Compile shaders
    auto vs = SketchBase::compile_shader(GL_VERTEX_SHADER, sweep_vert);
    auto fs = SketchBase::compile_shader(GL_FRAGMENT_SHADER, render_frag);

    // Link program, with position attribute
    render_prog = glCreateProgram();
    glAttachShader(render_prog, vs);
    glAttachShader(render_prog, fs);
    const GLint ixPosAttribute = 0;
    glBindAttribLocation(render_prog, ixPosAttribute, "position");
    glLinkProgram(render_prog);
    GLint ok = 0;
    glGetProgramiv(render_prog, GL_LINK_STATUS, &ok);
    if (!ok) SketchBase::throw_shader_link_error(render_prog);

    // Array buffer: for vertex array
    std::vector<GLfloat> quad;
    SketchBase::fill_quad(quad);

    glGenBuffers(1, &render_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quad.size(), &quad[0], GL_STATIC_DRAW);
}

void render()
{
    glUseProgram(render_prog);

    glBindBuffer(GL_ARRAY_BUFFER, render_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint tex_loc = glGetUniformLocation(render_prog, "tex");
    GLint resolution_loc = glGetUniformLocation(render_prog, "resolution");

    glUniform1i(tex_loc, 0);
    glUniform2f(resolution_loc, (float)W, (float)H);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, W, H);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFinish();
}
