#include "otter/trace-lookup-macros.h"
#include "otter/trace-structs.h"

#include "otter/queue.h"
#include "otter/stack.h"

/* pretty-print region definitions */
void
trace_region_pprint(
    FILE                *fp,
    trace_region_def_t  *r,
    const char func[],
    const int line)
{
    if (fp == NULL)
        fp = stderr;

    switch (r->type)
    {
    case trace_region_parallel:
        fprintf(fp, "%s:%d: Parallel(id=%lu, master=%lu, ref_count=%u, enter_count=%u) in %s:%d\n",
            __func__, __LINE__,
            r->attr.parallel.id,
            r->attr.parallel.master_thread,
            r->attr.parallel.ref_count,
            r->attr.parallel.enter_count,
            func, line
        );
        break;
    case trace_region_workshare:
        fprintf(fp, "%s:%d: Work(type=%s, count=%lu) in %s:%d\n",
            __func__, __LINE__,
            OMPT_WORK_TYPE_TO_STR(r->attr.wshare.type),
            r->attr.wshare.count,
            func, line
        );
        break;
    case trace_region_synchronise:
        fprintf(fp, "%s:%d: Sync(type=%s) in %s:%d\n",
            __func__, __LINE__,
            OMPT_SYNC_TYPE_TO_STR(r->attr.sync.type),
            func, line
        );
        break;
    case trace_region_task:
        fprintf(fp, "%s:%d: Task(id=%lu, type=%s) in %s:%d\n",
            __func__, __LINE__,
            r->attr.task.id,
            OMPT_TASK_TYPE_TO_STR(OMPT_TASK_TYPE_BITS & r->attr.task.type),
            func, line
        );
        break;
#if defined(USE_OMPT_MASKED)
    case trace_region_masked:
        fprintf(fp, "%s:%d: Masked(thread=%lu) in %s:%d\n",
#else
    case trace_region_master:
        fprintf(fp, "%s:%d: Master(thread=%lu) in %s:%d\n",
#endif
            __func__, __LINE__,
            r->attr.master.thread,
            func, line
        );
        break;
    }
    return;
}
