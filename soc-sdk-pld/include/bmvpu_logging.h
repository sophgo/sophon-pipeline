
#ifndef __BM_VPU_LOGGING_H__
#define __BM_VPU_LOGGING_H__

#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__)
#define ATTRIBUTE 
#define DECL_EXPORT __declspec(dllexport)
#define DECL_IMPORT __declspec(dllimport)
#else
#define ATTRIBUTE __attribute__((deprecated))
#define DECL_EXPORT
#define DECL_IMPORT
#endif
/* Log levels. */
typedef enum {
    VPU_LOG_ERROR   = 0,
    VPU_LOG_WARNING = 1,
    VPU_LOG_INFO    = 2,
    VPU_LOG_DEBUG   = 3, /* only usefull for developers */
    VPU_LOG_LOG     = 4, /* only usefull for developers */
    VPU_LOG_TRACE   = 5  /* only usefull for developers */
} vpu_log_level_t;

#define VPU_ERROR_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (vpu_log_level_threshold >= VPU_LOG_ERROR)   { vpu_logging("ERROR",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define VPU_WARNING_FULL(FILE_, LINE_, FUNCTION_, ...) do { if (vpu_log_level_threshold >= VPU_LOG_WARNING) { vpu_logging("WARNING", FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define VPU_INFO_FULL(FILE_, LINE_, FUNCTION_, ...)    do { if (vpu_log_level_threshold >= VPU_LOG_INFO)    { vpu_logging("info",    FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define VPU_DEBUG_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (vpu_log_level_threshold >= VPU_LOG_DEBUG)   { vpu_logging("debug",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define VPU_LOG_FULL(FILE_, LINE_, FUNCTION_, ...)     do { if (vpu_log_level_threshold >= VPU_LOG_LOG)     { vpu_logging("log",     FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)
#define VPU_TRACE_FULL(FILE_, LINE_, FUNCTION_, ...)   do { if (vpu_log_level_threshold >= VPU_LOG_TRACE)   { vpu_logging("trace",   FILE_, LINE_, FUNCTION_, __VA_ARGS__); } } while(0)


#define VPU_ERROR(...)    VPU_ERROR_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define VPU_WARNING(...)  VPU_WARNING_FULL(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define VPU_INFO(...)     VPU_INFO_FULL   (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define VPU_DEBUG(...)    VPU_DEBUG_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define VPU_LOG(...)      VPU_LOG_FULL    (__FILE__, __LINE__, __func__, __VA_ARGS__)
#define VPU_TRACE(...)    VPU_TRACE_FULL  (__FILE__, __LINE__, __func__, __VA_ARGS__)

extern vpu_log_level_t vpu_log_level_threshold;
DECL_IMPORT void vpu_set_logging_threshold(vpu_log_level_t threshold);
DECL_IMPORT void vpu_logging(char const *lvlstr, char const *file, int const line, char const *fn, const char *format, ...);

#endif /* __BM_VPU_LOGGING_H__ */

