/**
 * @file otter-task-graph.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Interface to Otter task graph event source API for recording task
 * graph via annotations.
 * @version 0.2.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 * @todo Implement threadsafe variable storing last task context created.
 * @todo Add loop context object to otterLoopBegin/End calls.
 * @todo Add loop context object to otterLoopIterationBegin/End calls.
 */

#if !defined(OTTER_TASK_GRAPH_H)
#define OTTER_TASK_GRAPH_H

// Only want the otter_task_context typedef rather than the interface itself.
// Interface only needed by the implementation of this event source.
#include "api/otter-task-graph/otter-task-context.h"

/**
 * @brief Convenience macro function for use with functions that require file,
 * func & line arguments.
 * 
 */
#define OTTER_SRC_ARGS() __FILE__, __func__, __LINE__

/**
 * @brief Indicates whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 * @see otterSynchroniseTasks()
 * 
 */
typedef enum otter_task_sync_t {
    otter_sync_children,
    otter_sync_descendants
} otter_task_sync_t;

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

/**
 * @brief Indicate the start of a region representing a task.
 * 
 * Indicate that the code enclosed by a matching `otterTaskEnd()` represents a
 * task which could be scheduled on some thread. Note that this does not mean
 * that the enclosed code is actually a task, rather that it could/should be a
 * task after parallelisation.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterTaskEnd()` call enclosing a region which could/
 *   should be a task.
 * - No synchronisation constraints are applied by default. To indicate that
 *   a task should be synchronised, see `otterSynchroniseTasks()`.
 * 
 * ## Semantics
 * 
 * Creates a unique representation of this task and records a `task-switch` 
 * event which switches from the encountering task to the new task. If the
 * parent task is non-NULL, the new task is created as a child of the parent.
 * Otherwise, the new task is an orphan task with no parent.
 * 
 * @warning if a task is falsely created as an orphan when there is infact a 
 * parent (a false orphan), it will not be possible to separate the duration of
 * the orphan from that of the true parent task i.e. the execution time of the
 * true parent will silently include the execution time of the false orphan.
 * 
 * @param file The path to the source file containing this call.
 * @param func The name of the enclosing function.
 * @param line The line at which this call appears.
 * @param parent_task The context representing the parent task. If NULL, the new
 * task is created as an orphan task (i.e. with no parent).
 * 
 * @returns A pointer to a otter_task_context which represents the new task
 */
otter_task_context *otterTaskBegin(const char* file, const char* func, int line, otter_task_context *parent_task);


/**
 * @brief Counterpart to `otterTaskBegin()`, indicating the end of a region 
 * representing a task.
 * 
 * @param task The context representing the completed task which was returned by
 * `otterTaskBegin()`.
 *
 * @see `otterTaskBegin()`
 */
void otterTaskEnd(otter_task_context *task);


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
