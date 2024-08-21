#include <stdarg.h>
#include <assert.h>

#include "pw_log.h"
#include "pw_arena.h"

static Pw_Logger LOGGER;

static const char* LOG_LEVEL_2_STR[] = {
    [PW_LOG_DEBUG] = "DEBUG",
    [PW_LOG_INFO] = "INFO",
    [PW_LOG_WARNING] = "WARNING",
    [PW_LOG_ERROR] = "ERROR",
};

Pw_Logger pw_logger_init(const Pw_Log_Level log_level, const char *filename)
{
    FILE *log_file = fopen(filename, "w");
    assert(log_file != NULL);

    LOGGER.log_level = log_level;
    LOGGER.log_file = log_file;
    LOGGER.scratchpad = pw_arena_init(1024);

    Pw_Logger logger = {
        .log_level = log_level,
        .log_file = log_file,
        .arena = pw_arena_init(1024),
        .scratchpad = pw_arena_init(1024),
    };
    return logger;
}

Pwstr get_prefix(const Pw_Log_Level log_level, const Pw_Loginfo info, Pw_Arena *arena, Pw_Arena scratchpad)
{
    const char * log_level_str = LOG_LEVEL_2_STR[log_level];
    return pwstr_format(arena, scratchpad, "%s: %s:%zu", log_level_str, info.file, info.line);
}

static void logger_vlog(Pw_Logger logger, const Pw_Log_Level log_level, const Pw_Loginfo info, const char * fmt, va_list arg)
{
    if (log_level == PW_LOG_OFF || log_level > logger.log_level) {
        return;
    }
    assert(log_level < PW_LOG_LEVEL_LEN && "Invalid log level");

    pw_arena_reset(&logger.arena);
    Pwstr prefix = get_prefix(log_level, info, &logger.arena, logger.scratchpad);
    Pwstr msg = pwstr_vformat(&logger.arena, logger.scratchpad, fmt, arg);
    fprintf(logger.log_file, "%s: %s\n", prefix.data, msg.data);
}


void pw_logger_log(Pw_Logger logger, const Pw_Log_Level log_level, const Pw_Loginfo info, const char * fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    logger_vlog(logger, log_level, info, fmt, argptr);
    va_end(argptr);
}

void pw_log(Pw_Log_Level log_level, const Pw_Loginfo info, const char * fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    logger_vlog(LOGGER, log_level, info, fmt, argptr);
    va_end(argptr);
}
