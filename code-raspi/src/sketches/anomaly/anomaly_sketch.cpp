#include "anomaly_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global

AnomalySketch::AnomalySketch(int w, int h, GLuint render_fbo)
    : FragSketch(w, h, render_fbo, anomaly_frag)
{
}
