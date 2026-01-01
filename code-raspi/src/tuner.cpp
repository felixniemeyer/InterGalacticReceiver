#include "tuner.h"

// Local dependencies
#include "error.h"
#include "lock.h"

// Global
#include <algorithm>
#include <math.h>
#include <string.h>

static const int tuned_enter = 2;
static const int tuned_leave = 8;
static const int near_enter = 14;
static const int near_leave = 20;

Tuner::Tuner(bool debug_log)
    : debug_log(debug_log)
    , val_buf(val_buf_sz)
    , val_buf_cpy(val_buf_sz)
{
    int r = pthread_mutex_init(&mut, NULL);
    if (r != 0)
        THROWF("Failed to initialize mutex: %d: %s", r, strerror(r));
}

void Tuner::add_station(int freq)
{
    Lock lock(&mut);

    int val = freq_to_val(freq);
    station_vals.push_back(val);

    if (debug_log) printf("Station %.1f added: value is %d\n", freq * 0.1, val);

    // TODO: Verify station is at least 2 * near_leave away from every existing one

    // Find lowest station and put us far below it. Next update will sort things out.
    int ix = 0;
    for (int i = 0; i < (int)station_vals.size(); ++i)
    {
        if (station_vals[i] < station_vals[ix]) ix = i;
    }
}

void Tuner::get_status(int &ix, TuneStatus &status)
{
    Lock lock(&mut);

    ix = station_ix;
    status = station_status;
}

void Tuner::update(int val)
{
    Lock lock(&mut);

    val = smooth_reading(val);

    if (station_vals.size() == 0) return;

    int station_val = station_vals[station_ix];
    int station_dist = abs(station_val - val);

    // Find nearest station
    int ix = -1;
    int dist = -1;
    for (int i = 0; i < (int)station_vals.size(); ++i)
    {
        int dist_here = abs(station_vals[i] - val);
        if (ix == -1 || dist_here < dist)
        {
            ix = i, dist = dist_here;
        }
    }
    // If new nearest station is significantly closer than current station: switch to it
    if (dist < station_dist - 6)
    {
        station_ix = ix;
        station_val = station_vals[station_ix];
        station_dist = dist;
        station_status = station_val >= val ? tsFarAbove : tsFarBelow;
    }
    int delta = val - station_val;
    TuneStatus last_status = station_status;

    // See if our status around station changes (getting closer, or farther away; with hysteresis)
    if (station_status == tsFarAbove || station_status == tsFarBelow)
    {
        if (dist <= tuned_enter) station_status = tsTuned;
        else if (dist <= near_enter) station_status = delta > 0 ? tsAbove : tsBelow;
    }
    else if (station_status == tsAbove || station_status == tsBelow)
    {
        if (dist >= near_leave) station_status = delta > 0 ? tsFarAbove : tsFarBelow;
        else if (dist <= tuned_enter) station_status = tsTuned;
    }
    else if (station_status == tsTuned)
    {
        if (dist >= near_leave) station_status = delta > 0 ? tsFarAbove : tsFarBelow;
        else if (dist >= tuned_leave) station_status = delta > 0 ? tsAbove : tsBelow;
    }

    if (last_status != station_status && debug_log)
    {
        printf("Status %2d -> %2d at dist %3d\n", last_status, station_status, dist);
    }
}

int Tuner::smooth_reading(int val)
{
    val_buf[val_ix++] = val;
    if (val_ix == val_buf_sz) val_ix = 0;
    val_buf_cpy.assign(val_buf.begin(), val_buf.end());
    std::sort(val_buf_cpy.begin(), val_buf_cpy.end());
    return val_buf_cpy[val_buf_sz / 2];
}

int Tuner::val_to_freq(int val)
{
    // Known values for Lagrange interpolation:
    // 144 =>  90 MHz !! off
    // 148 =>  90 MHz
    // 473 =>  98 MHz !! off
    // 470 =>  98 MHz
    // 703 => 102 MHz !! wrong
    // 750 => 104 MHz

    const double x1 = 148, y1 = 90;
    const double x2 = 470, y2 = 98;
    const double x3 = 750, y3 = 104;

    double x = val;

    // clang-format off
    double freq = y1 * ((x - x2)*(x - x3)) / ((x1 - x2)*(x1 - x3))
                + y2 * ((x - x1)*(x - x3)) / ((x2 - x1)*(x2 - x3))
                + y3 * ((x - x1)*(x - x2)) / ((x3 - x1)*(x3 - x2));
    // clang-format on

    return (int)round(freq * 10);
}

int Tuner::freq_to_val(int freq)
{
    // Known values for Lagrange interpolation:
    // 144 =>  90 MHz !! off
    // 148 =>  90 MHz
    // 473 =>  98 MHz !! off
    // 470 =>  98 MHz
    // 703 => 102 MHz !! wrong
    // 750 => 104 MHz

    const double y1 = 148, x1 = 90;
    const double y2 = 470, x2 = 98;
    const double y3 = 750, x3 = 104;

    double x = freq * 0.1;

    // clang-format off
    double val  = y1 * ((x - x2)*(x - x3)) / ((x1 - x2)*(x1 - x3))
                + y2 * ((x - x1)*(x - x3)) / ((x2 - x1)*(x2 - x3))
                + y3 * ((x - x1)*(x - x2)) / ((x3 - x1)*(x3 - x2));
    // clang-format on

    return (int)round(val);
}
