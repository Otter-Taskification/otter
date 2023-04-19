#pragma once
#include "otter-task-graph.h"

/**
 * @brief Provides classes which wrap the otter-task-graph API.
 * 
 */
namespace otter {

    /**
     * @brief Represents an Otter task context. The default constructor records 
     * an `otterTaskBegin` event for a task with no parent i.e. a root task.
     * Calling `task.make_child()` constructs a child of `task` and records
     * this in the trace.
     * 
     * Copying a task is disabled as it doesn't make sense to copy a task - each
     * task context represents a unique instance of some logical task.
     * 
     * Move-assignment is disabled as it doesn't make sense to overwrite one 
     * task's context with the moved context of another task.
     * 
     */
    class Task {

    public:

        /**
         * @brief Construct a root Task object i.e. a task with no parent,
         * recording `otterTaskBegin` in the trace.
         * 
         */
        Task(void);

        /**
         * @brief Construct a child task from its parent, recording 
         * `otterTaskBegin` in the trace.
         * 
         * @return Task: the new child task.
         */
        Task make_child(void);

        /**
         * @brief Move-construct a new task by adopting the context from another
         * task.
         * 
         * @param other: the task whose context is moved into the new task.
         */
        Task(Task&& other);

        // Doesn't make sense to copy or move-assign a task:
        Task(const Task& other)             = delete;   // copy-constructor
        Task& operator=(const Task& other)  = delete;   // copy-assignment
        Task& operator=(Task&& other)       = delete;   // move-assignment

        /**
         * @brief Record a synchronisation constraint on the descendants of a
         * task.
         *
         * @param mode: whether the synchronisation constraint applies to child
         * tasks only or all descendant tasks.
         */
        void synchronise_tasks(otter_task_sync_t mode);

        /**
         * @brief Explicitly end the task, recording `otterTaskEnd` in the
         * trace. No effect if the task was already ended.
         * 
         */
        void end_task(void);

        /**
         * @brief Destroy the Task object, calling `task.end_task()` if the task
         * wasn't already ended.
         * 
         */
        ~Task(void);

    private:

        /**
         * @brief The underlying Otter task context handle.
         * 
         */
        otter_task_context *m_task_context = nullptr;

        /**
         * @brief Construct a new Task object from its parent's context,
         * recording `otterTaskBegin` in the trace.
         * 
         * @param parent 
         */
        Task(otter_task_context* parent);
    };

    /**
     * @brief A singleton class representing the global context for Otter. Must 
     * be declared before any other Otter functions or classes are used. Stores 
     * a handle to a root task which may be used as the ancestor of all tasks
     * in the trace.
     * 
     */
    class Otter {
    public:

        /**
         * @brief Return the handle to the static global Otter context.
         * Initialises the trace with `otterTraceInitialise` on first use. Must
         * be declared before any other Otter classes or functions can be used.
         * 
         * @return Otter&
         */
        static Otter& get_otter(void);
        
        /**
         * @brief Delete the root task and finalise the Otter trace with
         * `otterTraceFinalise`.
         * 
         */
        ~Otter(void);

        Otter(const Otter&)             = delete;
        Otter(Otter&&)                  = delete;
        Otter& operator=(const Otter&)  = delete;
        Otter& operator=(Otter&&)       = delete;

        /**
         * @brief Get the root task context.
         * 
         * @return Task& 
         */
        Task& get_root_task(void);

    private:

        /**
         * @brief Initialise the trace and create a root task. Private to prevent
         * initialisation other than in the static `get` method.
         * 
         */
        Otter(void);

        /**
         * @brief The handle to the trace's root task.
         * 
         */
        Task* m_root_task;
    };

} // namespace otter
