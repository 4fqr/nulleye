#ifndef NYE_AI_FEATURES_H
#define NYE_AI_FEATURES_H

#include "core/event_bus.h"

#define NYE_FEATURE_COUNT 6

void ai_extract_features(const nuleye_event_t *ev, int out[NYE_FEATURE_COUNT]);

#endif