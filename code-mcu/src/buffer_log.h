#ifndef BUFFER_LOG
#define BUFFER_LOG

#include <stdint.h>

struct BufferLog
{
    static const uint8_t bufSz = 50;
    volatile uint16_t buf[bufSz] = {0};
    volatile mutable uint16_t buf2[bufSz] = {0};
    volatile bool full = false;
    volatile uint8_t ix = 0;

    inline void logValue(uint16_t val) volatile
    {
        buf[ix] = val;
        ix = (ix + 1) % bufSz;
        if (ix == 0) full = true;
    }

    uint16_t BufferLog::getAvg() volatile const;
};

#endif
