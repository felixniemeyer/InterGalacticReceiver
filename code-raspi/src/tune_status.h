#ifndef TUNE_STATUS
#define TUNE_STATUS

enum TuneStatus
{
    tsFarAbove = 2,
    tsAbove = 1,
    tsTuned = 0,
    tsBelow = -1,
    tsFarBelow = -2,
    tsNone = 99,
};

#endif
