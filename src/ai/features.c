#include "ai/features.h"
#include <string.h>
#include <time.h>

void ai_extract_features(const nuleye_event_t *ev, int out[NYE_FEATURE_COUNT])
{
    if (!ev || !out) return;
    out[0] = (int)(ev->pid & 0xFFF);
    out[1] = (int)(ev->ppid & 0xFFF);
    out[2] = (int)(ev->uid);
    out[3] = (int)strlen(ev->comm);
    out[4] = (int)strlen(ev->path);
    time_t ts = (time_t)(ev->ts / 1000000000ULL);
    struct tm tm;
    localtime_r(&ts, &tm);
    out[5] = tm.tm_hour;
}
