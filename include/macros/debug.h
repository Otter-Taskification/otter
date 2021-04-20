#if !defined(DEBUG_MACROS_H)
#define DEBUG_MACROS_H

/* detect whether __VA_OPT__ supported 
   credit: https://stackoverflow.com/a/48045656
*/
#define THIRD_ARG(a,b,c,...) c
#define VA_OPT_AVAIL_I(...) THIRD_ARG(__VA_OPT__(,),1,0,)
#define VA_OPT_AVAIL VA_OPT_AVAIL_I(?)

/* my addition to make variadic macros agnostic of __VA_OPT__ support */
#if VA_OPT_AVAIL
#define PASS_ARGS_I(...) __VA_OPT__(,) __VA_ARGS__
#else
#define PASS_ARGS_I(...) , ##__VA_ARGS__
#endif

#define PASS_ARGS(...) PASS_ARGS_I(__VA_ARGS__)

#include <stdio.h>

#if !defined(DEBUG_LEVEL)
#define DEBUG_LEVEL 0
#endif

#define LOG_ERROR(fmt, ...)                      \
    fprintf(stderr, "[E] " fmt " (%s:%d in %s)\n"  \
        PASS_ARGS(__VA_ARGS__), __FILE__, __LINE__, __func__)

#define LOG_ERROR_IF(pred, fmt, ...)             \
    do { if (pred) LOG_ERROR(fmt, __VA_ARGS__); } while(0)

#if DEBUG_LEVEL > 0
#define LOG_WARN(fmt, ...)                       \
    fprintf(stderr, "[w] " fmt " (%s:%d in %s)\n"  \
        PASS_ARGS(__VA_ARGS__), __FILE__, __LINE__, __func__)
#define LOG_WARN_IF(pred, fmt, ...)              \
    do { if (pred) LOG_WARN(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_WARN(...)
#define LOG_WARN_IF(...)
#endif

#if DEBUG_LEVEL > 1
#define LOG_INFO(fmt, ...)                       \
    fprintf(stderr, "[i] " fmt " (%s:%d in %s)\n"  \
        PASS_ARGS(__VA_ARGS__), __FILE__, __LINE__, __func__)
#define LOG_INFO_IF(pred, fmt, ...)              \
    do { if (pred) LOG_INFO(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_INFO(...)
#define LOG_INFO_IF(...)
#endif

#if DEBUG_LEVEL > 2
#define LOG_DEBUG(fmt, ...)                      \
    fprintf(stderr, "[d] " fmt " (%s:%d in %s)\n"  \
        PASS_ARGS(__VA_ARGS__), __FILE__, __LINE__, __func__)
#define LOG_DEBUG_IF(pred, fmt, ...)             \
    do { if (pred) LOG_DEBUG(fmt, __VA_ARGS__); } while(0)
#else
#define LOG_DEBUG(...)
#define LOG_DEBUG_IF(...)
#endif  

#endif // DEBUG_MACROS_H
