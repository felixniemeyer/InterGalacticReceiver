#include "star_sketch.h"

#include "horrors.h"

// GLSL
#include "shaders.h"

// Global

StarSketch::StarSketch(int w, int h, GLuint render_fbo)
    : FragSketch(w, h, render_fbo, star_frag)
{
}
