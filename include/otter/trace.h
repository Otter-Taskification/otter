#if !defined(OTTER_TRACE_H)
#define OTTER_TRACE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <otf2/otf2.h>

#include "otter/otter-ompt-header.h"
#include "otter/otter-common.h"

#define DEFAULT_LOCATION_GRP 0 // OTF2_UNDEFINED_LOCATION_GROUP
#define DEFAULT_SYSTEM_TREE  0
#define DEFAULT_NAME_BUF_SZ  256

#define CHECK_OTF2_ERROR_CODE(r)                                               \
    {if (r != OTF2_SUCCESS)                                                    \
    {                                                                          \
        LOG_ERROR("%s: %s (%s:%d)",                                            \
            OTF2_Error_GetName(r),                                             \
            OTF2_Error_GetDescription(r),                                      \
            __FILE__,                                                          \
            __LINE__                                                           \
        );                                                                     \
    }}

#define get_unique_rgn_ref() (get_unique_uint32_ref(trace_region))
#define get_unique_str_ref() (get_unique_uint32_ref(trace_string))
#define get_unique_loc_ref() (get_unique_uint64_ref(trace_location))
#define get_other_ref()      (get_unique_uint64_ref(trace_other))

/* Different kinds of unique IDs */
typedef enum trace_ref_type_t {
    trace_region,
    trace_string,
    trace_location,
    trace_other,
    NUM_REF_TYPES // <- MUST BE LAST ENUM ITEM
} trace_ref_type_t;

/* event endpoint */
typedef enum {
    trace_event_type_enter,
    trace_event_type_leave,
    trace_event_type_task_create
} trace_event_type_t;

/* Different kinds of regions supported */
typedef enum {
    trace_region_parallel,
    trace_region_workshare,
    trace_region_synchronise,
    trace_region_task,
#if defined(USE_OMPT_MASKED)
    trace_region_masked
#else
    trace_region_master
#endif
} trace_region_type_t;

/* Defined in trace-structs.h */
typedef struct trace_region_def_t trace_region_def_t;
typedef struct trace_location_def_t trace_location_def_t;

/* unique OTF2 refs accessed via macro wrappers */
uint64_t get_unique_uint64_ref(trace_ref_type_t ref_type);
uint32_t get_unique_uint32_ref(trace_ref_type_t ref_type);

/* interface function prototypes */
// bool trace_initialise_archive(otter_opt_t *opt);
// bool trace_finalise_archive(void);

/* write events */
void trace_event_thread_begin(trace_location_def_t *self);
void trace_event_thread_end(trace_location_def_t *self);
void trace_event_enter(trace_location_def_t *self, trace_region_def_t *region);
void trace_event_leave(trace_location_def_t *self);
void trace_event_task_create(trace_location_def_t *self, trace_region_def_t *created_task);
void trace_event_task_schedule(trace_location_def_t *self, trace_region_def_t *prior_task, ompt_task_status_t prior_status);
void trace_event_task_switch(trace_location_def_t *self, trace_region_def_t *prior_task, ompt_task_status_t prior_status, trace_region_def_t *next_task);
// void trace_event_task_complete(trace_location_def_t *self);

/* write definitions to the global def writer */
void trace_write_location_definition(trace_location_def_t *loc);
void trace_write_region_definition(trace_region_def_t *rgn);

#endif // OTTER_TRACE_H
