#ifndef NYE_FILE_INTEGRITY_H
#define NYE_FILE_INTEGRITY_H

#include "modules/base.h"

extern nuleye_module_t file_integrity_module;

int file_integrity_schedule_scan(const char *path);

#endif
