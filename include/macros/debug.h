#if !defined(DEBUG_MACROS_H)
#define DEBUG_MACROS_H

#include <stdio.h>

#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#define LOG_ERROR(fmt, ...)                      \
    fprintf(stderr, "[E] (%s:%s:%d) " fmt "\n",  \
        __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

#define LOG_ERROR_IF(pred, fmt, ...)             \
    do { if (pred) LOG_ERROR(fmt, __VA_ARGS__); } while(0)

#if DEBUG_LEVEL > 0
#define LOG_WARN(fmt, ...)                       \
    fprintf(stderr, "[w] (%s:%s:%d) " fmt "\n",  \
        __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_WARN_IF(pred, fmt, ...)              \
    do { if (pred) LOG_WARN(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_WARN(...)
#define LOG_WARN_IF(...)
#endif

#if DEBUG_LEVEL > 1
#define LOG_INFO(fmt, ...)                       \
    fprintf(stderr, "[i] (%s:%s:%d) " fmt "\n",  \
        __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_INFO_IF(pred, fmt, ...)              \
    do { if (pred) LOG_INFO(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_INFO(...)
#define LOG_INFO_IF(...)
#endif

#if DEBUG_LEVEL > 2
#define LOG_DEBUG(fmt, ...)                      \
    fprintf(stderr, "[d] (%s:%s:%d) " fmt "\n",  \
        __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define LOG_DEBUG_IF(pred, fmt, ...)             \
    do { if (pred) LOG_DEBUG(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_DEBUG(...)
#define LOG_DEBUG_IF(...)
#endif  

#endif // DEBUG_MACROS_H
