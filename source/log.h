#pragma once

#include <stdint.h>

#define LOG_NI_ERROR    0
#define LOG_APP_ERROR   1
#define LOG_LOG         2

int log_init(int verbosity_level, const char *log_path);
void log_free(void);

//don't use directly; use macros defined below
void _log_add(int verbose_level, const char *format, ...);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#define log_ni_error(format, args...)   _log_add(LOG_NI_ERROR, format, ## args);
#define log_app_error(format, args...)  _log_add(LOG_APP_ERROR, format, ## args);
#define log(format, args...)            _log_add(LOG_LOG, format, ## args);
#pragma GCC diagnostic pop
