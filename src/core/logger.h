#ifndef NYE_LOGGER_H
#define NYE_LOGGER_H

#include <stdarg.h>

typedef enum { NYE_LOG_DEBUG = 0, NYE_LOG_INFO = 1, NYE_LOG_WARN = 2, NYE_LOG_ERR = 3 } nye_log_level_t;

int logger_init(const char *path);
void logger_fini(void);
void nulleye_log(nye_log_level_t level, const char *fmt, ...);

#endif
