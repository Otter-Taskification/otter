/* 
    This file stores definitions for attribute names and labels that are
    included into trace.c using macros that are defined as-needed. This makes
    it easy to add/change attributes without needing to repeat & update code
    in multiple places. It works in this way:

    trace.c:
        >>>>>>
        typedef enum {
        #define INCLUDE_ATTRIBUTE(Type, Name, Desc)\   <- define macro here
            attr_##Name,
        #define INCLUDE_LABEL(...)                     <- define macro here
        #include "otf2-attribute-defs.h"               <- macro invoked here
        #undef INCLUDE_ATTRIBUTE
        #undef INCLUDE_LABEL                           <- macros undefined
            n_attr_defined
        } attr_name_enum_t;
        <<<<<<

        This gives an enum type defined dynamically at compile-time by the
        contents of this header

 */

#if !defined(INCLUDE_LABEL)
#define INCLUDE_LABEL(...)     // noop
#endif

#if !defined(INCLUDE_ATTRIBUTE)
#define INCLUDE_ATTRIBUTE(...) // noop
#endif

INCLUDE_LABEL(flag,  Y )
INCLUDE_LABEL(flag,  N )
INCLUDE_LABEL(flag,  true  )
INCLUDE_LABEL(flag,  false )

/* Unique ID attributes */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, unique_id, "unique ID of a task, parallel region or thread")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, prior_task_id, "unique ID of a task suspended at a task-scheduling point")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, next_task_id, "unique ID of a task resumed at a task-scheduling point")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, encountering_task_id, "unique ID of the task that encountered this region")

/* Attributes relating to parallel regions */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT32, requested_parallelism, "requested parallelism of parallel region")
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, is_league, "is this parallel region a league of teams?")

/* Attributes relating to workshare regions (sections, single, loop, taskloop) */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, workshare_type, "type of workshare region")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, workshare_count, "number of iterations associated with workshare region")

/* Attributes relating to sync regions (barrier, taskgroup, taskwait) */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, sync_type, "type of synchronisation region")

/* Attributes relating to task regions */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, parent_task_id, "unique ID of the parent task of this task")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT32, task_flags, "flags set for this task")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_has_dependences, "whether this task has dependences")

/* Attributes defined for all events */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, event_type, "type of event as defined by Otter")
INCLUDE_LABEL(event_type,  thread_begin   )
INCLUDE_LABEL(event_type,  thread_end     )
INCLUDE_LABEL(event_type,  parallel_begin )
INCLUDE_LABEL(event_type,  parallel_end   )
INCLUDE_LABEL(event_type,  workshare_begin)
INCLUDE_LABEL(event_type,  workshare_end  )
INCLUDE_LABEL(event_type,  sync_begin     )
INCLUDE_LABEL(event_type,  sync_end       )
INCLUDE_LABEL(event_type,  task_create    )
INCLUDE_LABEL(event_type,  task_switch    )
INCLUDE_LABEL(event_type,  task_enter     )
INCLUDE_LABEL(event_type,  task_leave     )
INCLUDE_LABEL(event_type,  master_begin   )
INCLUDE_LABEL(event_type,  master_end     )

/* Result of call to sched_getcpu() */
INCLUDE_ATTRIBUTE(OTF2_TYPE_INT32, cpu, "cpu on which the encountering thread is running")

/* Region begin or end event? */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, endpoint, "is this a region-enter or region-leave event")
INCLUDE_LABEL(endpoint, enter   )
INCLUDE_LABEL(endpoint, leave   )
INCLUDE_LABEL(endpoint, discrete)

/* task type */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_type, "task classification")
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, parent_task_type, "task classification of the parent task of this task")
INCLUDE_LABEL(task_type,  initial_task  )
INCLUDE_LABEL(task_type,  implicit_task )
INCLUDE_LABEL(task_type,  explicit_task )
INCLUDE_LABEL(task_type,  target_task   )

/* task flags */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_is_undeferred, "task is undeferred")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_is_untied,     "task is untied")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_is_final,      "task is final")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_is_mergeable,  "task is mergeable")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_is_merged,     "task is merged")

/* thread type */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, thread_type, "thread type")
INCLUDE_LABEL(thread_type,  initial)
INCLUDE_LABEL(thread_type,  worker )

/* region type - parallel, workshare, sync, task */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, region_type, "region type")
/* generic region types */
INCLUDE_LABEL(region_type, parallel)
INCLUDE_LABEL(region_type, workshare)
INCLUDE_LABEL(region_type, sync)
INCLUDE_LABEL(region_type, task)
/* task region sub-types */
INCLUDE_LABEL(region_type, initial_task)
INCLUDE_LABEL(region_type, implicit_task)
INCLUDE_LABEL(region_type, explicit_task)
INCLUDE_LABEL(region_type, target_task)
/* workshare region sub-types */
INCLUDE_LABEL(region_type, sections)
INCLUDE_LABEL(region_type, single_executor)
INCLUDE_LABEL(region_type, single_other)
INCLUDE_LABEL(region_type, distribute)
INCLUDE_LABEL(region_type, loop)
INCLUDE_LABEL(region_type, taskloop)
INCLUDE_LABEL(region_type, master)
/* sync region sub-types */
INCLUDE_LABEL(region_type, barrier)
INCLUDE_LABEL(region_type, barrier_implicit)
INCLUDE_LABEL(region_type, barrier_explicit)
INCLUDE_LABEL(region_type, barrier_implementation)
INCLUDE_LABEL(region_type, taskwait)
INCLUDE_LABEL(region_type, taskgroup)

/* prior task status at task-schedule event */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, prior_task_status,  "status of the task that arrived at a task scheduling point")
INCLUDE_LABEL(prior_task_status,  complete      )
INCLUDE_LABEL(prior_task_status,  yield         )
INCLUDE_LABEL(prior_task_status,  cancel        )
INCLUDE_LABEL(prior_task_status,  detach        )
INCLUDE_LABEL(prior_task_status,  early_fulfil  )
INCLUDE_LABEL(prior_task_status,  late_fulfil   )
INCLUDE_LABEL(prior_task_status,  switch        )

#undef INCLUDE_LABEL
#undef INCLUDE_ATTRIBUTE
