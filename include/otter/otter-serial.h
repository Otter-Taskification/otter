/**
 * @file otter-serial.h
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Public API for the `otter-serial` event source
 * @version 0.1.0
 * @date 2022-05-19
 * 
 * @copyright Copyright (c) 2021, Adam Tuft. All rights reserved.
 * 
 * @todo finalise "simple trace" event source for use in single-threaded app (only with updates to current implementation of otter-serial)
 * @todo replace otterSynchroniseDescendantTasks[Begin|End] with macro/function barrier to synchronise child/descendant tasks
 * @todo (eventually) make otterParallel[Begin|End] optional (will require re-engineering otter-trace)
 * @todo (eventually) add the concept of "phases" -> probably depends on re-engineering of otter-trace
 * @todo (eventually) need to be able to start/stop tracing at will -> probably depends on re-engineering of otter-trace
 * @todo (long term) generic trace API that accepts diverse event sources from user code
 */

#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

#include "otter/trace-enum-types.h"


/**
 * @brief Defines whether a task synchronisation construct should apply a 
 * synchronisation constraint to immediate child tasks or all descendant tasks.
 * 
 * @see otterSynchroniseTasks()
 * 
 */
typedef trace_task_sync_t otter_task_sync_t;

/**
 * @brief Convenience macro function for use with functions that require file,
 * func & line arguments.
 * 
 */
#define OTTER_SRC_ARGS() __FILE__, __func__, __LINE__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise Otter overall
 *
 * This has to be the very first Otter function you invoke.
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
 * Sets up a trace and initialises Otter's internals ready to begin tracing.
 * Creates an internal representation of the thread executing the instrumented
 * application; this thread representation records all events dispatched to
 * Otter.
 * 
 * Implicitly records a `task-begin` event with an initial task representing the
 * region within which all other events are recorded.
 * 
 * @param file The path to the source file containing this call.
 * @param func The name of the enclosing function.
 * @param line The line in @param file at which this call appears.
 */
void otterTraceInitialise(const char* file, const char* func, int line);


/**
 * @brief Finalise Otter and safely close the trace.
 * 
 * Is the counterpart to `otterTraceInitialise()`.
 * 
 * @see `otterTraceInitialise()`
 */
void otterTraceFinalise(void);


/**
 * @brief Toggle tracing on
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
 * @brief Toggle tracing off
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
 * @brief Indicate the start of a region which could be executed in parallel.
 * 
 * Indicates that the code enclosed by a matching `otterThreadsEnd()` could be ran
 * in parallel across multiple threads. Note that this does NOT mean that the
 * code is actually run in parallel - this is merely a hint to Otter that the
 * enclosed code could be parallelised.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterThreadsEnd()` call enclosing a region which could/
 *   should be parallelised.
 * 
 * 
 * ## Semantics
 * 
 * Creates a representation of the enclosed region which will nest regions
 * created within it. Implicitly creates and begins a task within which the 
 * region is executed.
 * 
 * 
 * ## OpenMP
 *
 * Corresponds to the `#pragma omp parallel` where you create a team of threads
 * without distributing work to them. It would/could also correspond to a teams
 * statement where you create multiple teams, possible even on another device.
 * 
 * @param file The path to the source file containing this call.
 * @param func The name of the enclosing function.
 * @param line The line in @param file at which this call appears.
 */
void otterThreadsBegin(const char* file, const char* func, int line);


/**
 * @brief Indicate the end of a region which could be executed in parallel.
 * 
 * Counterpart to `otterThreadsBegin()`.
 *
 * @see `otterThreadsBegin()`
 */
void otterThreadsEnd(void);


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
 *   a task should be synchronised, see `otterSynchroniseTasks()` or
 *   `otterSynchroniseDescendantTasksBegin()`.
 * 
 * ## Semantics
 * 
 * Creates a unique representation of this task and records a `task-create`
 * event immediately followed by a `task-switch` event which switches from the
 * encountering task to the new task.
 * 
 * @param file The path to the source file containing this call.
 * @param func The name of the enclosing function.
 * @param line The line in @param file at which this call appears.
 */
void otterTaskBegin(const char* file, const char* func, int line);


/**
 * @brief Indicate the end of a region representing a task
 * 
 * Counterpart to `otterTaskBegin()`.
 *
 * @see `otterTaskBegin()`
 */
void otterTaskEnd(void);


/**
 * @brief Indicate the beginning of a loop.
 * 
 * Indicate that code enclosed by a matching `otterLoopEnd()` call represents a 
 * loop which could/should be run in parallel. Note that this does NOT mean that
 * the enclosed code is already parallelised
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterLoopEnd()` call to indicate the end of the loop
 *   region.
 * 
 * ## Semantics
 * 
 * Creates a representation of this loop and records a `loop-begin` event.
 * 
 */
void otterLoopBegin(void);


/**
 * @brief Indicate the end of a loop.
 * 
 * Counterpart to `otterLoopBegin()`.
 * 
 * @see `otterLoopBegin()`
 * 
 */
void otterLoopEnd(void);

/**
 * @brief Indicate the beginning of a loop iteration. [TODO]
 * 
 * Indicate that code enclosed by a matching `otterLoopIterationEnd()` represents
 * one iteration of an enclosing loop.
 * 
 * @warning This function is currently a stub; it is defined but has yet to be
 * fully implemented.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a corresponding `otterLoopIterationEnd()` call.
 * 
 * 
 * ## Semantics
 * 
 * - None (stub function).
 * 
 */
void otterLoopIterationBegin(void);


/**
 * @brief Indicate the end of a loop iteration. [TODO]
 * 
 * Counterpart to `otterLoopIterationBegin()`.
 * 
 * @warning This function is currently a stub; it is defined but has yet to be
 * fully implemented.
 * 
 */
void otterLoopIterationEnd(void);


/**
 * @brief Indicate the start of a region which should be executed by exactly one
 * thread in a team. 
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterTaskSingleEnd()` call to indicate the end of the
 *   region.
 * 
 * 
 * ## Semantics
 * 
 * Creates a representation of this region and records a `single-begin` event.
 * 
 * 
 * ## OpenMP
 * 
 * This is analogous to the `#pragma omp single` construct i.e. this snippet:
 * 
 *     #pragma omp parallel
 *     {
 *         #pragma omp single
 *         {
 *             // Executted by one thread in the team
 *         }
 *     }
 * 
 * is analogous to this snippet:
 * 
 *     otterThreadsBegin();
 *         otterTaskSingleBegin();
 *             // Should be executed by one thread in the team.
 *         otterTaskSingleEnd();
 *     otterThreadEnd();
 * 
 * 
 */
void otterTaskSingleBegin(void);


/**
 * @brief Indicate the end of a region which should be executed by exactly one
 * thread in a team.
 * 
 * Counterpart to `otterTaskSingleBegin()`.
 * 
 * @see `otterTaskSingleBegin()`
 * 
 */
void otterTaskSingleEnd(void);


/**
 * @brief Indicate a synchronisation constraint on the children of the
 * encountering task
 * 
 * 
 * ## Usage
 * 
 * - This is a freestanding operation which indicates that the encountering
 *   task may not proceed until all prior child tasks are complete.
 * - Note that this constraint applies to immediate child tasks only, not their
 *   descendants.
 * 
 * 
 * ## Semantics
 * 
 * Creates a region representing a barrier which waits on child tasks. Records
 * an enter event immediately followed by a leave event.
 * 
 * 
 * ## OpenMP
 * 
 * Analogous to the `#pragma omp taskwait` directive.
 * 
 */
void otterSynchroniseTasks(otter_task_sync_t mode);


/**
 * @brief Indicate the start of a region within which all descendant tasks of
 * the encountering task will synchronise.
 * 
 * @deprecated This paired construct is deprecated and will be replaced by a 
 * freestanding operation.
 * 
 * Indicate a region, bound by a matching `otterSynchroniseDescendantTasksEnd()`
 * call, within which all descendant tasks must be complete by the end of the
 * region.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `otterSynchroniseDescendantTasksEnd()` call to indicate
 *   the end of the region.
 * 
 * 
 * ## Semantics
 * 
 * Creates and immediately enters a region within which this synchronisation
 * constraint should be applied.
 * 
 * 
 * ## OpenMP
 * 
 * Analogous to the `#pragma omp taskgroup` construct.
 * 
 */
void otterSynchroniseDescendantTasksBegin(void);


/**
 * @brief Indicate the end of a region within which all descendant tasks of the
 * encountering task will synchronise.
 * 
 * Counterpart to `otterSynchroniseDescendantTasksBegin()`
 * 
 * @deprecated This paired construct is deprecated and will be replaced by a 
 * freestanding operation.
 * 
 * @see `otterSynchroniseDescendantTasksBegin()`
 */
void otterSynchroniseDescendantTasksEnd(void);


/**
 * @brief Start a new algorithmic phase [TODO]
 *
 * By default, all trace events fall into the same global phase. However, some
 * codes run through particular phases and will want to study these phases
 * independently. With the present routine you mark the begin of such a phase.
 * Each phase has to be given a unique name.
 * 
 * @warning This feature is not yet implemented.
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
 */
void otterPhaseBegin( const char* name );


/**
 * @brief End the present algorithmic phase [TODO]
 * 
 * Indicates the end of the present algorithmic phase and return to the
 * encountering task.
 * 
 * @warning This feature is not yet implemented.
 * 
 * @see `otterPhaseBegin()`
 * @see `otterPhaseSwitch()`
 * 
 */
void otterPhaseEnd();


/**
 * @brief End the present algorithmic phase and immediately proceed to another.
 * [TODO]
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
 */
void otterPhaseSwitch( const char* name );


#ifdef __cplusplus
}
#endif

#endif // OTTER_SERIAL_H
