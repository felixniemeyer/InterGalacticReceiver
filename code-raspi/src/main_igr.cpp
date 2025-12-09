#include "main.h"

// Local dependencies
#include "error.h"
#include "fps.h"
#include "hardware_controller.h"
#include "horrors.h"
#include "magic.h"
#include "render_blender.h"
#include "sketch_base.h"
#include "tuner.h"

// Sketches
#include "sketches/mmgl01/mmgl01_sketch.h"
#include "sketches/star/star_sketch.h"

// Global
#include <vector>

static Tuner tuner(false);
static std::vector<SketchBase *> sketches;
static int sketch_ix = -1;

static void init_stations(GLuint render_fbo);
static void update_station(RenderBlender &renderer, double current_time);

void main_igr()
{
    RenderBlender renderer;
    init_stations(renderer.fbo());

    HardwareController::set_listeners(&tuner);
    HardwareController::init();

    FPS fps(TARGET_FPS);
    double last_time = fps.frame_start();

    while (app_running)
    {
        double current_time = fps.frame_start();
        double dt = current_time - last_time;
        last_time = current_time;

        update_station(renderer, current_time);
        if (sketch_ix == -1) continue;

        sketches[sketch_ix]->frame(dt);
        renderer.render(current_time);
        put_on_screen();
        fps.frame_end();
    }
}

template <typename T>
void add_station(GLuint render_fbo, int freq)
{
    auto sketch = new T(W, H, render_fbo);
    sketch->init();
    tuner.add_station(freq);
    sketches.push_back(sketch);
}

void init_stations(GLuint render_fbo)
{
    add_station<StarSketch>(render_fbo, 980);
    add_station<MMGL01Sketch>(render_fbo, 967);
}

void update_station(RenderBlender &renderer, double current_time)
{
    int station_ix;
    TuneStatus tuner_status;
    tuner.get_status(station_ix, tuner_status);

    if (station_ix < -1) return;

    if (station_ix != sketch_ix && sketch_ix != -1)
    {
        sketches[sketch_ix]->unload(current_time);
        sketches[station_ix]->reload(current_time);
    }
    sketch_ix = station_ix;

    if (tuner_status == tsTuned)
        renderer.set_mode(bmSketch);
    else if (tuner_status == tsAbove || tuner_status == tsBelow)
        renderer.set_mode(bmInfo);
    else renderer.set_mode(bmStatic);
}
