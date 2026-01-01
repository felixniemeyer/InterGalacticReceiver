#ifndef RAY_SKETCH_H
#define RAY_SKETCH_H

#include "sketch_frag.h"
#include <vector>

class RaySketch : public FragSketch
{
  private:
    uint8_t *bg_pixels = nullptr;
    unsigned int bg_w;
    unsigned int bg_h;
    GLuint bg_tex = 0;

  public:
    RaySketch(int w, int h, GLuint render_fbo);
    // void frame(double dt) override;
    void init() override;
    void unload(double current_time) override;
    void reload(double current_time) override;
};

#endif
