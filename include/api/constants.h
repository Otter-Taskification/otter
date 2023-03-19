/**
 * @brief Defines whether a task synchronisation construct should apply a
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 *
 * @see otterSynchroniseTasks()
 *
 */
typedef enum {
    otter_sync_global,
    otter_sync_children,
    otter_sync_descendants
} otter_sync_t;
