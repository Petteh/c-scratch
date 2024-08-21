#ifndef PW_LOG_H
#define PW_LOG_H

#include <stdio.h>

#include "pw_string.h"

typedef enum {
    PW_LOG_OFF,
    PW_LOG_DEBUG,
    PW_LOG_INFO,
    PW_LOG_WARNING,
    PW_LOG_ERROR,
    PW_LOG_LEVEL_LEN
} Pw_Log_Level;

typedef struct {
    Pw_Log_Level log_level;
    FILE *log_file;
    Pw_Arena arena;
    Pw_Arena scratchpad;
} Pw_Logger;

typedef struct {
    const char * file;
    const size_t line;
} Pw_Loginfo;

Pw_Logger pw_logger_init(const Pw_Log_Level log_level, const char *filename);
void pw_logger_log(const Pw_Logger logger, const Pw_Log_Level log_level, const Pw_Loginfo info, const char * fmt, ...);
void pw_log(const Pw_Log_Level log_level, const Pw_Loginfo info, const char * fmt, ...);

#define PW_LOG(log_level, fmt, ...)   pw_log(log_level, (Pw_Loginfo) {.file = __FILE__, .line = __LINE__}, fmt, __VA_ARGS__)

#define PW_LOG_DEBUG(fmt, ...)      PW_LOG(PW_LOG_DEBUG,    fmt, __VA_ARGS__)
#define PW_LOG_INFO(fmt, ...)       PW_LOG(PW_LOG_INFO,     fmt, __VA_ARGS__)
#define PW_LOG_WARNING(fmt, ...)    PW_LOG(PW_LOG_WARNING,  fmt, __VA_ARGS__)
#define PW_LOG_ERROR(fmt, ...)      PW_LOG(PW_LOG_ERROR,    fmt, __VA_ARGS__)


#endif // PW_LOG_H
