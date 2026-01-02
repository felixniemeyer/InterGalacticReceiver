#ifndef TUNING_FEEDBACK_H
#define TUNING_FEEDBACK_H

#include "tune_status.h"
#include <stdint.h>

class TuningFeedback
{
  private:
    uint32_t last_changed_at = 0;
    TuneStatus prev_status = tsNone;

  private:
    static uint32_t get_msec();

  public:
    TuningFeedback();
    void tune_status(TuneStatus status);
};

#endif
