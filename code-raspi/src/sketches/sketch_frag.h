#ifndef SKETCH_FRAG_H
#define SKETCH_FRAG_H

#include "sketch_base.h"
#include <vector>

class FragSketch : public SketchBase
{
  private:
    const int w, h;
    const GLuint render_fbo;
    const char *frag;
    GLuint vs = 0;
    GLuint fs = 0;
    GLuint prog = 0;
    GLuint vbo = 0;
    double time;

  public:
    FragSketch(int w, int h, GLuint render_fbo, const char *frag);
    void init() override;
    void frame(double dt) override;
    void unload() override;
    void reload(double elapsed) override;
};

#endif
