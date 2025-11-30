#include "main.h"

// Local dependencies
#include "error.h"
#include "fps.h"
#include "horrors.h"
#include "magic.h"
#include "sketch_base.h"
#include "tuner.h"

// Sketches
#include "sketches/star/star_sketch.h"

// Global
#include <vector>

static Tuner tuner;
static std::vector<SketchBase *> sketches;
static int sketch_ix = -1;

static void init_stations();

void main_icr()
{
    init_stations();
    FPS fps(TARGET_FPS);
    double last_time = fps.frame_start();
    while (app_running)
    {
        double current_time = fps.frame_start();
        double dt = current_time - last_time;
        sketches[sketch_ix]->frame(dt);
        put_on_screen();
        fps.frame_end();
    }
}

void init_stations()
{
    auto star_sketch = new StarSketch(W, H);
    star_sketch->init();
    sketches.push_back(star_sketch);
    sketch_ix = 0;
}
