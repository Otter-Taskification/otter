/**
 * @file otter-task-graph.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Interface to Otter task graph event source API for recording task
 * graph via annotations.
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 */

#if !defined(OTTER_TASK_GRAPH_H)
#define OTTER_TASK_GRAPH_H

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

/**
 * @brief Forward declaration of the otter_task_context struct.
 * 
 */
typedef struct otter_task_context otter_task_context;

typedef struct {
    const char* file;
    const char* func;
    int         line;
} otter_source_args;

/**
 * @brief Indicates whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 * @see otterSynchroniseTasks
 * 
 */
typedef enum otter_task_sync_t {
    otter_sync_children,
    otter_sync_descendants
} otter_task_sync_t;

/**
 * @brief Used to indicate whether a task should be added to a given task pool.
 * 
 * @see otterTaskInitialise
 * 
 */
typedef enum otter_add_to_pool_t {
    otter_no_add_to_pool,
    otter_add_to_pool
} otter_add_to_pool_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise Otter overall
 *
 * This has to be the very first Otter function you invoke. It must be called
 * strictly in a serial environment i.e. with 1 thread.
 * 
 * 
 * ## Usage
 * 
 * - This routine must precede all other otter* routines.
 * - The counterpart `otterTraceFinalise()` must be called immediately prior to
 *   program termination in order to safely close the trace.
 * - This routine implicitly starts event tracing, which can be deactivated with
 *   `otterTraceStop()`.
 * 
 * 
 * ## Semantics
 * 
 * Does the following ONLY:
 *  - Sets up a trace and initialises Otter's internals ready to begin tracing.
 * 
 */
void otterTraceInitialise(void);


/**
 * @brief The counterpart to `otterTraceInitialise()`. Finalise Otter and safely
 * close the trace.
 * 
 * @see `otterTraceInitialise()`
 */
void otterTraceFinalise(void);


/**
 * @brief Toggle tracing on.
 *
 * The tracing is automatically started by `otterTraceInitialise()` but you can
 * stop it any time with `otterTraceStop()` and then resume it via this function.
 * 
 * @warning toggling tracing on/off at different levels of the call tree may
 * result in an ill-formed trace. Otter does NOT check that you have started/
 * stopped tracing at a sensible point.
 *
 * @see `otterTraceStop()`
 * @see `otterTraceInitialise()`
 */
void otterTraceStart(void);


/**
 * @brief Toggle tracing off.
 *
 * To re-activate the tracing, you have to call `otterTraceStart()`.
 * 
 * @warning toggling tracing on/off at different levels of the call tree may
 * result in an ill-formed trace. Otter does NOT check that you have started/
 * stopped tracing at a sensible point.
 * 
 * @see `otterTraceStart()`
 * @see `otterTraceInitialise()`
 */
void otterTraceStop(void);


/******
 * Defining Tasks
 ******/


/**
 * @brief Initialise a task handle with the given flavour as a child of parent.
 * If `push_task` is true, the task will be stored internally under a label
 * given by format and any subsequent arguments.
 * 
 * @warning if a task is falsely created as an orphan when there is infact a 
 * parent (a false orphan), it will not be possible to separate the duration of
 * the orphan from that of the true parent task i.e. the execution time of the
 * true parent will silently include the execution time of the false orphan.
 * 
 * @note Does not record any events in the trace.
 * 
 * @param parent_task: The handle of the parent of the new task.
 * @param flavour: The user-defined flavour of the new task.
 * @param push_task: Whether to associate the task with the given label.
 * @param init: The source location where the task was initialised.
 * @param format: the format of the label, using subsequent arguments.
 * 
 */
otter_task_context *otterTaskInitialise(otter_task_context *parent_task, int flavour, otter_add_to_pool_t add_to_pool, otter_source_args init, const char *format, ...);


/******
 * Annotating Task Start & End
 ******/


/**
 * @brief Record the start of a region which represents previously initialised
 * task.
 * 
 * Indicate that the code enclosed by a matching `otterTaskEnd()` represents a
 * task which could be scheduled. Note that this does not mean that the enclosed
 * code is actually a task, rather that it could/should be a task after
 * parallelisation.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterTaskEnd()` call enclosing a region which could/
 *   should be a task.
 * - No synchronisation constraints are recorded by default. To indicate that
 *   a task should be synchronised, see `otterSynchroniseTasks()`.
 * 
 * ## Semantics
 * 
 * Records a `task-switch` event at the current source location which indicates
 * a switch from the encountering task to the new task.
 * 
 * @param task The handle to the task representing the annotated region of code.
 * @param start The source location at which the task was started.
 * 
 * @returns A pointer to a otter_task_context which represents the started task
 */
otter_task_context *otterTaskStart(otter_task_context *task, otter_source_args start);

/**
 * @brief Counterpart to `otterTaskStart()`, indicating the end of the code 
 * representing the given task.
 * 
 * @param task The completed task.
 * @param end The source location at which the task ended.
 *
 * @see `otterTaskStart()`
 */
void otterTaskEnd(otter_task_context *task, otter_source_args end);


/******
 * Registering & Retrieving Tasks
 ******/


/**
 * @brief Associate the given task with the label. The task can later be 
 * retrieved by `otterTaskGetLabel`.
 * 
 * @param task The task to associate with the label
 * @param format The format of the label, used to format subsequent arguments.
 * 
 */
void otterTaskPushLabel(otter_task_context *task, const char *format, ...);

/**
 * @brief Pop the task which was previously registered with the given label.
 * Returns NULL if no such task exists. Further attempts to get or pop the task
 * for this label will return NULL until/unless another task is registered
 * with this label.
 * 
 * @param format The format of the label, used to format subsequent arguments.
 * 
 */
otter_task_context *otterTaskPopLabel(const char *format, ...);

/**
 * @brief Borrow a task which was previously registered with the given label.
 * Returns NULL if no such task exists. Note that the caller does not own the
 * borrowed task handle.
 * 
 * @param format a format string controlling the formatting of a label using the
 * subsequent arguments.
 */
otter_task_context *otterTaskBorrowLabel(const char *format, ...);


/******
 * Annotating Task Synchronisation Constraints
 ******/


/**
 * @brief Indicate a synchronisation constraint on the children or descendants
 * of the encountering task. If no encountering task is specified, assume that
 * the synchronisation constraint applies to all orphan tasks (and their 
 * descendants) previously created and not already synchronised.
 * 
 * 
 * ## Usage
 * 
 * - This is a freestanding operation which indicates that the encountering
 *   task may not proceed until all prior child or descendant tasks are complete.
 * - The `mode` argument indicates whether this constraint applies to immediate
 *   children only or to all descendant tasks.
 * 
 * 
 * ## Semantics
 * 
 * Creates a record in the trace that the encountering task met a 
 * synchronisation constraint which applies its child or descendant tasks which
 * were previously created.
 * 
 * @param task The context for the task which encountered the synchronisation
 * contraint. The constraint applies to the scheduling of child or descendants
 * of the encountering task. If NULL, assume that the constraint applies to all
 * orphan tasks previously created and not already synchronised.
 * @param mode Indicates whether this barrier synchronises immediate children of
 * the encountering task (`otter_sync_children`) or all descendants of the
 * encountering task (`otter_sync_descendants`).
 * 
 */
void otterSynchroniseTasks(otter_task_context *task, otter_task_sync_t mode);


/******
 * Managing Phases
 ******/


/**
 * @brief Start a new algorithmic phase
 *
 * By default, all trace events fall into the same global phase. However, some
 * codes run through particular phases and will want to study these phases
 * independently. With the present routine you mark the begin of such a phase.
 * Each phase has to be given a unique name.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a corresponding `otterPhaseEnd()`.
 * 
 * 
 * ## Semantics
 * 
 * Creates a meta-region to nest all other regions encountered within it. Phases
 * may themselves be nested.
 * 
 * 
 * @param name A unique identifier for this phase
 * 
 * @see `otterPhaseEnd()`
 * @see `otterPhaseSwitch()`
 * 
 * @todo Probably want to introduce an `otterPhaseContext` type for book-keeping
 * during phase.
 * 
 */
void otterPhaseBegin( const char* name );


/**
 * @brief End the present algorithmic phase
 * 
 * Indicates the end of the present algorithmic phase and return to the
 * encountering task.
  * 
 * @see `otterPhaseBegin()`
 * @see `otterPhaseSwitch()`
 * 
 * @todo Should probably accept some sort of `otterPhaseContext` for easier
 * book-keeping.
 * 
 */
void otterPhaseEnd();


/**
 * @brief End the present algorithmic phase and immediately proceed to another.
 * 
 * Indicates the end of the present algorithmic phase and the immediate start
 * of another.
 * 
 * Equivalent to:
 * 
 *     otterPhaseEnd();
 *     otterPhaseBegin(...);
 * 
 * @param name The name of the next phase to begin
 * 
 * @see `otterPhaseBegin()`
 * @see `otterPhaseEnd()`
 * 
 * @todo Should probably accept & return some sort of `otterPhaseContext` for 
 * easier book-keeping.
 * 
 */
void otterPhaseSwitch( const char* name );


#ifdef __cplusplus
}
#endif

#endif // OTTER_TASK_GRAPH_H
