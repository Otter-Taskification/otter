#if !defined(OMPT_COMMON_H)
#define OMPT_COMMON_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t unique_id_t;

typedef struct otter_opt_t {
    char    *hostname;
    char    *graph_output;
    char    *graph_format;
    char    *graph_nodeattr;
    bool     append_hostname;
} otter_opt_t;

#endif // OMPT_COMMON_H