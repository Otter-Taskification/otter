#if !defined(OTTER_COMMON_H)
#define OTTER_COMMON_H

/* 
    Definitions common to multiple modules
 */

#include <stdint.h>
#include <stdbool.h>

// Bits used by ompt_task_flag_t to indicate task type
#define OMPT_TASK_TYPE_BITS 0x0F

typedef uint64_t unique_id_t;

typedef struct otter_opt_t {
    char    *hostname;
    char    *tracename;
    char    *tracepath;
    char    *archive_name;
    bool     append_hostname;
} otter_opt_t;

#endif // OTTER_COMMON_H
