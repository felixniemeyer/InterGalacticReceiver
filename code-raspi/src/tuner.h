#ifndef TUNER_H
#define TUNER_H

#include "tune_status.h"
#include "value_listener_if.h"
#include <pthread.h>
#include <stdint.h>
#include <vector>

class Tuner : public IValueListener
{
  private:
    const bool debug_log;
    pthread_mutex_t mut;
    std::vector<int> station_vals;
    static const int val_buf_sz = 2;
    std::vector<int> val_buf;
    std::vector<int> val_buf_cpy;
    int val_ix = 0;
    int station_ix = -1;
    TuneStatus station_status = tsNone;

  private:
    int smooth_reading(int val);

  public:
    static int val_to_freq(int val);
    static int freq_to_val(int freq);
    Tuner(bool debug_log);
    void update(int val) override;
    void add_station(int freq);
    void get_status(int &ix, TuneStatus &status);
};

#endif
