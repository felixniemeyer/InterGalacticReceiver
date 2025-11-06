#include "buffer_log.h"

#include <stdlib.h>
#include <string.h>

static int cmp_uint16(const void *a, const void *b)
{
    uint16_t ua = *(const uint16_t *)a;
    uint16_t ub = *(const uint16_t *)b;
    return (ua > ub) - (ua < ub);
}

uint16_t BufferLog::getAvg() volatile const
{
    uint32_t sum = 0;
    if (!full)
    {
        for (uint8_t i = 0; i < ix; ++i)
            sum += buf[i];
        return (sum + ix / 2) / ix;
    }

    // Discard outliers: 10 on both sides
    const uint8_t discardBand = 10;
    memcpy((void *)buf2, (void *)buf, sizeof(uint16_t) * bufSz);
    qsort((void *)buf2, bufSz, sizeof(uint16_t), cmp_uint16);
    uint8_t sz = bufSz - 2 * discardBand;
    uint16_t max = 0;
    for (uint8_t i = discardBand; i < bufSz - discardBand; ++i)
    {
        sum += buf[i];
        if (buf[i] > max) max = buf[i];
    }
    // return max;
    return (sum + sz / 2) / sz;
}
