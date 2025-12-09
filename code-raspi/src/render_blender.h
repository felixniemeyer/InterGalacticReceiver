#ifndef RENDER_BLENDER_H
#define RENDER_BLENDER_H

#include <GLES2/gl2.h>

enum BlendMode
{
    bmStatic,
    bmInfo,
    bmSketch,
};

class RenderBlender
{
  private:
    GLuint render_tex = 0;
    GLuint render_depth = 0;
    GLuint render_fbo = 0;
    GLuint render_prog = 0;
    GLuint render_vbo = 0;
    BlendMode mode = bmStatic;

  private:
    void init_render_target();
    void compile_render_prog();

  public:
    RenderBlender();
    GLuint fbo() const { return render_fbo; }
    void set_mode(BlendMode mode);
    void render(double time);
};

#endif
