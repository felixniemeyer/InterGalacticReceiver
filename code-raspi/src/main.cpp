// Local dependencies
#include "canvas_ity.h"
#include "gfx_helpers.h"
#include "hardware_controller.h"
#include "magic.h"

// Global
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <libgen.h>
#include <linux/fb.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static uint8_t *font_data;
static size_t font_data_size;

static bool light_on = false;
static const int rbsz = 4;
static int rbuf[rbsz] = {0};
static int rpos = 0;

int main()
{
    HardwareController::init();

    canvas_ity::canvas ctx(W, H);
    float *image = new float[H * W * 4];
    char buf[64];

    font_data = load_canvas_font(&font_data_size);
    if (font_data == nullptr)
    {
        fprintf(stderr, "Failed to load font; quitting\n");
        return -1;
    }
    ctx.set_font(font_data, font_data_size, 64);

    while (true)
    {
        int tuner, aknob, bknob, cknob, swtch;
        HardwareController::get_values(tuner, aknob, bknob, cknob, swtch);

        if (swtch < 4 && !light_on)
        {
            HardwareController::set_light(true);
            light_on = true;
        }
        else if (swtch > 12 && light_on)
        {
            HardwareController::set_light(false);
            light_on = false;
        }

        rbuf[rpos] = tuner;
        rpos = (rpos + 1) % rbsz;
        int sum = 0;
        for (int i = 0; i < rbsz; ++i)
            sum += rbuf[i];
        int avg = (sum + rbsz / 2) / rbsz;

        int freq = tuner_val_to_freq(tuner);

        // printf("\rTuner  %4d  SW %4d   ", tuner, swtch);

        usleep(100000);

        ctx.clear();

        ctx.set_line_width(6.0f);
        ctx.set_color(canvas_ity::stroke_style, 0.95, 0.65, 0.15, 1.0);
        ctx.begin_path();
        ctx.arc(360, 250, 70, 0, M_PI * 2);
        ctx.stroke();

        ctx.set_color(canvas_ity::fill_style, 0.8, 0.8, 0.8, 1);
        sprintf(buf, "Tuner %5d", tuner);
        ctx.fill_text(buf, 100, 100);
        sprintf(buf, "Freq  %5d", freq);
        ctx.fill_text(buf, 100, 164);

        sprintf(buf, "    A  %4d", aknob);
        ctx.fill_text(buf, 100, 228);
        sprintf(buf, "    B  %4d", bknob);
        ctx.fill_text(buf, 100, 292);
        sprintf(buf, "    C  %4d", cknob);
        ctx.fill_text(buf, 100, 356);
        sprintf(buf, "   SW  %4d", swtch);
        ctx.fill_text(buf, 100, 428);

        ctx.get_image_data(image, W, H);
        flush_to_fb(image);
    }
}
