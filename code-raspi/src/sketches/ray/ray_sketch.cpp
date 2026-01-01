#include "ray_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global

static const char *bg_file_name = "img-tile-warm.png";

RaySketch::RaySketch(int w, int h, GLuint render_fbo)
    : FragSketch(w, h, render_fbo, ray_frag)
{
    load_png(&bg_pixels, &bg_w, &bg_h, bg_file_name);
}

// void RaySketch::frame(double dt)
// {
//     time += dt;

//     glUseProgram(prog);

//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     // This is redundant, but it's what we'll need if attributes change frame-by-frame
//     glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quad.size(), &quad[0], GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

//     GLint time_loc = glGetUniformLocation(prog, "time");
//     GLint resolution_loc = glGetUniformLocation(prog, "resolution");

//     glUniform1f(time_loc, (float)time);
//     glUniform2f(resolution_loc, (float)w, (float)h);

//     glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
//     glViewport(0, 0, w, h);
//     glClearColor(0, 0, 0, 1);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//     glDrawArrays(GL_TRIANGLES, 0, 6);
//     glFinish();
// }

void RaySketch::init()
{
    FragSketch::init();
    bg_tex = create_texture(bg_pixels, bg_w, bg_h);
}

void RaySketch::unload(double current_time)
{
    glDeleteTextures(1, &bg_tex);
    bg_tex = 0;
    FragSketch::unload(current_time);
}

void RaySketch::reload(double current_time)
{
    // This in turn call my init() override, so nothing else to do here
    FragSketch::reload(current_time);
}