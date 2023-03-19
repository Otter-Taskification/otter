#if !defined(OTTER_OMPT_HEADER_H)
#include "otter/otter-ompt-header.h"
#endif

#if defined(implements_callback_thread_begin)
static void
on_ompt_callback_thread_begin(
    ompt_thread_t            thread_type,
    ompt_data_t             *thread
);
#endif

#if defined(implements_callback_thread_end)
static void
on_ompt_callback_thread_end(
    ompt_data_t             *thread
);
#endif

#if defined(implements_callback_parallel_begin)
static void
on_ompt_callback_parallel_begin(
    ompt_data_t             *encountering_task,
    const ompt_frame_t      *encountering_task_frame,
    ompt_data_t             *parallel,
    unsigned int             requested_parallelism,
    int                      flags,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_parallel_end)
static void
on_ompt_callback_parallel_end(
    ompt_data_t             *parallel,
    ompt_data_t             *encountering_task,
    int                      flags,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_task_create)
static void
on_ompt_callback_task_create(
    ompt_data_t             *encountering_task,
    const ompt_frame_t      *encountering_task_frame,
    ompt_data_t             *new_task,
    int                      flags,
    int                      has_dependences,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_task_schedule)
static void
on_ompt_callback_task_schedule(
    ompt_data_t             *prior_task,
    ompt_task_status_t       prior_task_status,
    ompt_data_t             *next_task
);
#endif

#if defined(implements_callback_implicit_task)
static void
on_ompt_callback_implicit_task(
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    unsigned int             actual_parallelism,
    unsigned int             index,
    int                      flags
);
#endif

#if defined(implements_callback_target)
static void
on_ompt_callback_target(
    ompt_target_t            kind,
    ompt_scope_endpoint_t    endpoint,
    int                      device_num,
    ompt_data_t             *task,
    ompt_id_t                target_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_target_data_op)
static void
on_ompt_callback_target_data_op(
    ompt_id_t                target_id,
    ompt_id_t                host_op_id,
    ompt_target_data_op_t    optype,
    void                    *src_addr,
    int                      src_device_num,
    void                    *dest_addr,
    int                      dest_device_num,
    size_t                   bytes,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_target_submit)
static void
on_ompt_callback_target_submit(
    ompt_id_t                target_id,
    ompt_id_t                host_op_id,
    unsigned int             requested_num_teams
);
#endif

#if defined(implements_callback_device_initialize)
static void
on_ompt_callback_device_initialize(
    int                      device_num,
    const char              *type,
    ompt_device_t           *device,
    ompt_function_lookup_t   lookup,
    const char              *documentation
);
#endif

#if defined(implements_callback_device_finalize)
static void
on_ompt_callback_device_finalize(
    int                      device_num
);
#endif

#if defined(implements_callback_device_load)
static void
on_ompt_callback_device_load(
    int                      device_num,
    const char              *filename,
    int64_t                  offset_in_file,
    void                    *vma_in_file,
    size_t                   bytes,
    void                    *host_addr,
    void                    *device_addr,
    uint64_t                 module_id
);
#endif

#if defined(implements_callback_device_unload)
static void
on_ompt_callback_device_unload(
    int                      device_num,
    uint64_t                 module_id
);
#endif

#if defined(implements_callback_sync_region_wait)
static void
on_ompt_callback_sync_region_wait(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_mutex_released)
static void
on_ompt_callback_mutex_released(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_dependences)
static void
on_ompt_callback_dependences(
    ompt_data_t             *task,
    const ompt_dependence_t *deps,
    int                      ndeps
);
#endif

#if defined(implements_callback_task_dependence)
static void
on_ompt_callback_task_dependence(
    ompt_data_t             *src_task,
    ompt_data_t             *sink_task
);
#endif

#if defined(implements_callback_work)
static void
on_ompt_callback_work(
    ompt_work_t              wstype,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    uint64_t                 count,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_master)
#if defined(USE_OMPT_MASKED)
static void
on_ompt_callback_masked(
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra
);
#else
static void
on_ompt_callback_master(
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra
);
#endif
#endif

#if defined(implements_callback_target_map)
static void
on_ompt_callback_target_map(
    ompt_id_t                target_id,
    unsigned int             nitems,
    void *                  *host_addr,
    void *                  *device_addr,
    size_t                  *bytes,
    unsigned int            *mapping_flags,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_sync_region)
static void
on_ompt_callback_sync_region(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_lock_init)
static void
on_ompt_callback_lock_init(
    ompt_mutex_t             kind,
    unsigned int             hint,
    unsigned int             impl,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_lock_destroy)
static void
on_ompt_callback_lock_destroy(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_mutex_acquire)
static void
on_ompt_callback_mutex_acquire(
    ompt_mutex_t             kind,
    unsigned int             hint,
    unsigned int             impl,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_mutex_acquired)
static void
on_ompt_callback_mutex_acquired(
    ompt_mutex_t             kind,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_nest_lock)
static void
on_ompt_callback_nest_lock(
    ompt_scope_endpoint_t    endpoint,
    ompt_wait_id_t           wait_id,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_flush)
static void
on_ompt_callback_flush(
    ompt_data_t             *thread,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_cancel)
static void
on_ompt_callback_cancel(
    ompt_data_t             *task,
    int                      flags,
    const void              *codeptr_ra
);
#endif

#if defined(implements_callback_reduction)
static void
on_ompt_callback_reduction(
    ompt_sync_region_t       kind,
    ompt_scope_endpoint_t    endpoint,
    ompt_data_t             *parallel,
    ompt_data_t             *task,
    const void              *codeptr_ra
);
#endif

