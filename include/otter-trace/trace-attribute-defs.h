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
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, encountering_task_id, "unique ID of the task that encountered this region")

/* Attributes relating to parallel regions */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT32, requested_parallelism, "requested parallelism of parallel region")
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, is_league, "is this parallel region a league of teams?")

/* Attributes relating to workshare regions (sections, single, loop, taskloop) */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, workshare_type, "type of workshare region")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, workshare_count, "number of iterations associated with workshare region")

/* Attributes relating to sync regions (barrier, taskgroup, taskwait) */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, sync_type, "type of synchronisation region")
// INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, sync_encountering_task_id, "the task a thread was executing when it encountered the sync region")

/* Attributes relating to task regions */
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT64, parent_task_id, "unique ID of the parent task of this task")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT32, task_flags, "flags set for this task")
INCLUDE_ATTRIBUTE(OTF2_TYPE_UINT8, task_has_dependences, "whether this task has dependences")

/* Attributes defined for all events */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, event_type, "type of event as defined by Otter")
INCLUDE_LABEL(event_type,  parallel_begin )
INCLUDE_LABEL(event_type,  parallel_end   )
INCLUDE_LABEL(event_type,  workshare_begin)
INCLUDE_LABEL(event_type,  workshare_end  )
INCLUDE_LABEL(event_type,  sync_begin     )
INCLUDE_LABEL(event_type,  sync_end       )
INCLUDE_LABEL(event_type,  task_create    )
INCLUDE_LABEL(event_type,  task_schedule  )
INCLUDE_LABEL(event_type,  task_enter     )
INCLUDE_LABEL(event_type,  task_leave     )

/* task type */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_type, "task classification")
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, parent_task_type, "task classification of the parent task of this task")
INCLUDE_LABEL(task_type,  initial  )
INCLUDE_LABEL(task_type,  implicit )
INCLUDE_LABEL(task_type,  explicit )
INCLUDE_LABEL(task_type,  target   )

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
INCLUDE_LABEL(region_type, parallel)
INCLUDE_LABEL(region_type, sections)
INCLUDE_LABEL(region_type, single_executor)
INCLUDE_LABEL(region_type, single_other)
INCLUDE_LABEL(region_type, workshare)
INCLUDE_LABEL(region_type, distribute)
INCLUDE_LABEL(region_type, loop)
INCLUDE_LABEL(region_type, taskloop)
INCLUDE_LABEL(region_type, barrier)
INCLUDE_LABEL(region_type, barrier_implicit)
INCLUDE_LABEL(region_type, barrier_explicit)
INCLUDE_LABEL(region_type, barrier_implementation)
INCLUDE_LABEL(region_type, taskwait)
INCLUDE_LABEL(region_type, taskgroup)
INCLUDE_LABEL(region_type, task)

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
