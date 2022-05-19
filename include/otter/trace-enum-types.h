#if !defined(TRACE_ENUM_TYPES_H)
#define TRACE_ENUM_TYPES_H


/**
 * @brief Defines whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 */
typedef enum {
    sync_children,
    sync_descendants
} trace_task_sync_t;

#endif // TRACE_ENUM_TYPES_H
