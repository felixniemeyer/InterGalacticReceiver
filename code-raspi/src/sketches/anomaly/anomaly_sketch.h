#ifndef ANOMALY_SKETCH_H
#define ANOMALY_SKETCH_H

#include "sketch_frag.h"

class AnomalySketch : public FragSketch
{
  public:
    AnomalySketch(int w, int h, GLuint render_fbo);
};

#endif
