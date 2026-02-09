#include "bezix_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global

BezixSketch::BezixSketch(int w, int h, GLuint render_fbo)
    : FragSketch(w, h, render_fbo, bezix_frag)
{
}
