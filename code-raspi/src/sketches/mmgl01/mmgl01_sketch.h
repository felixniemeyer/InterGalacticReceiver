#ifndef MMGL01_SKETCH_H
#define MMGL01_SKETCH_H

#include "sketch_frag.h"
#include <vector>

class MMGL01Sketch : public FragSketch
{
  public:
    MMGL01Sketch(int w, int h, GLuint render_fbo);
};

#endif
