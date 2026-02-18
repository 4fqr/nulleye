#ifndef NYE_RULES_H
#define NYE_RULES_H

#include <stddef.h>

int rules_load_from_file(const char *path);
int rules_evaluate_and_act(const void *event_blob, size_t len);

#endif