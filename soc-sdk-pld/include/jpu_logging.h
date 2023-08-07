
#ifndef __JPU_LOGGING_H__
#define __JPU_LOGGING_H__

/* Log levels. */
typedef enum {
    JPU_LOG_ERROR   = 0,
    JPU_LOG_WARNING = 1,
    JPU_LOG_INFO    = 2,
    JPU_LOG_DEBUG   = 3,
    JPU_LOG_LOG     = 4,
    JPU_LOG_TRACE   = 5
} jpu_log_level_t;

#define JPU_ERROR_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (jpu_log_level_threshold >= JPU_LOG_ERROR)   { jpu_logging("ERROR",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define JPU_WARNING_FULL(FILE_, LINE_, FUNCTION_, ...) do { if (jpu_log_level_threshold >= JPU_LOG_WARNING) { jpu_logging("WARNING", FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define JPU_INFO_FULL(FILE_, LINE_, FUNCTION_, ...)    do { if (jpu_log_level_threshold >= JPU_LOG_INFO)    { jpu_logging("info",    FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define JPU_DEBUG_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (jpu_log_level_threshold >= JPU_LOG_DEBUG)   { jpu_logging("debug",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define JPU_LOG_FULL(FILE_, LINE_, FUNCTION_, ...)     do { if (jpu_log_level_threshold >= JPU_LOG_LOG)     { jpu_logging("log",     FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define JPU_TRACE_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (jpu_log_level_threshold >= JPU_LOG_TRACE)   { jpu_logging("trace",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)


#define JPU_ERROR(...)    JPU_ERROR_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define JPU_WARNING(...)  JPU_WARNING_FULL(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define JPU_INFO(...)     JPU_INFO_FULL   (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define JPU_DEBUG(...)    JPU_DEBUG_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define JPU_LOG(...)      JPU_LOG_FULL    (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define JPU_TRACE(...)    JPU_TRACE_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)

extern jpu_log_level_t jpu_log_level_threshold;
void jpu_set_logging_threshold(jpu_log_level_t threshold);
void jpu_logging(char const *lvlstr, char const *file, int const line, char const *fn, const char *format, ...);

#endif /* __JPU_LOGGING_H__ */

