#if !defined(TRACE_ENUM_TYPES_H)
#define TRACE_ENUM_TYPES_H

// TODO: these enum constants are actually part of the interface to otter-trace.
// Where they are exposed to the user, a module should wrap them in its own
// enum type so as not to expose the internal interface between otter modules.

/**
 * @brief Defines whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 */
typedef enum {
    trace_sync_children,
    trace_sync_descendants
} trace_task_sync_t;

#endif // TRACE_ENUM_TYPES_H
