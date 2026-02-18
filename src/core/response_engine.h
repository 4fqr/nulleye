#ifndef NYE_RESPONSE_ENGINE_H
#define NYE_RESPONSE_ENGINE_H

int response_engine_init(void);
void response_engine_fini(void);
int response_engine_perform(const char *action, const char *target, const char *reason);

#endif