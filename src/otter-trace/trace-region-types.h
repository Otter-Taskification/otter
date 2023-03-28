#if !defined(TRACE_REGION_TYPE_ENUM_H)
#define TRACE_REGION_TYPE_ENUM_H

/* Different kinds of regions supported */
typedef enum {
    trace_region_parallel,
    trace_region_workshare,
    trace_region_synchronise,
    trace_region_task,
#if defined(USE_OMPT_MASKED)
    trace_region_masked,
#else
    trace_region_master,
#endif
    trace_region_phase
} trace_region_type_t;

#endif
