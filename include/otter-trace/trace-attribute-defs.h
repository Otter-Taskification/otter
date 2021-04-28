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

INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_type,          task classification)
INCLUDE_LABEL(task_type,  initial  ) // initial
INCLUDE_LABEL(task_type,  implicit ) // implicit
INCLUDE_LABEL(task_type,  explicit ) // explicit
INCLUDE_LABEL(task_type,  target   ) // target

/* Y/N flags */
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_undeferred, task is undeferred)
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_untied,     task is untied)
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_final,      task is final)
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_mergeable,  task is mergeable)
INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, task_merged,     task is merged)

INCLUDE_ATTRIBUTE(OTF2_TYPE_STRING, prior_task_status,  status of the task that arrived at a task scheduling point)
INCLUDE_LABEL(prior_task_status,  complete      ) // complete
INCLUDE_LABEL(prior_task_status,  yield         ) // yield
INCLUDE_LABEL(prior_task_status,  cancel        ) // cancel
INCLUDE_LABEL(prior_task_status,  detach        ) // detach
INCLUDE_LABEL(prior_task_status,  early_fulfil  ) // early fulfil
INCLUDE_LABEL(prior_task_status,  late_fulfil   ) // late fulfil
INCLUDE_LABEL(prior_task_status,  switch        ) // switch

#undef INCLUDE_LABEL
#undef INCLUDE_ATTRIBUTE
