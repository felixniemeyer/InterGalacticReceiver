#ifndef STAR_SKETCH_H
#define STAR_SKETCH_H

#include "sketch_base.h"
#include <vector>

class StarSketch : public SketchBase
{
  private:
    const int w, h;
    GLuint vs = 0;
    GLuint fs = 0;
    GLuint prog = 0;
    GLuint vbo = 0;
    double time;

  public:
    StarSketch(int w, int h);
    void init() override;
    void frame(double dt) override;
    void unload() override;
    void reload(double elapsed) override;
};

#endif
