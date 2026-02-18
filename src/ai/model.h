#ifndef NYE_AI_MODEL_H
#define NYE_AI_MODEL_H

#include "ai/isolation_forest.h"

int ai_model_save(const char *key, iforest_t *f);
int ai_model_load(const char *key, iforest_t *f);

#endif