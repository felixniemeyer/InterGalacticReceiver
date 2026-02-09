#ifndef BEZIX_SKETCH_H
#define BEZIX_SKETCH_H

#include "sketch_frag.h"

class BezixSketch : public FragSketch
{
  public:
    BezixSketch(int w, int h, GLuint render_fbo);
};

#endif
