/**
 * @file otter-task-graph-user.h
 * @author Adam Tuft
 * @brief Provides macros for accessing the Otter task-graph API for the purpose
 * of annotating user code. These macros are exported by defining
 * 'OTTER_TASK_GRAPH_ENABLE_USER' immediately before including this file in each
 * source file where annotations appear.
 * @version 0.1
 * @date 2023-05-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#if !defined(OTTER_TASK_GRAPH_API_MACRO_H)
#define OTTER_TASK_GRAPH_API_MACRO_H

#if defined(OTTER_TASK_GRAPH_ENABLE_USER)
#include "otter-task-graph.h"

/* detect whether __VA_OPT__ supported 
   credit: https://stackoverflow.com/a/48045656
*/
#define OTTER_IMPL_THIRD_ARG(a,b,c,...) c
#define OTTER_IMPL_VA_OPT_AVAIL_I(...) OTTER_IMPL_THIRD_ARG(__VA_OPT__(,),1,0,)
#define OTTER_IMPL_VA_OPT_AVAIL OTTER_IMPL_VA_OPT_AVAIL_I(?)

/* my addition to make variadic macros agnostic of __VA_OPT__ support */
#if OTTER_IMPL_VA_OPT_AVAIL
#define OTTER_IMPL_PASS_ARGS_I(...) __VA_OPT__(,) __VA_ARGS__
#else
#define OTTER_IMPL_PASS_ARGS_I(...) , ##__VA_ARGS__
#endif
#define OTTER_IMPL_PASS_ARGS(...) OTTER_IMPL_PASS_ARGS_I(__VA_ARGS__)

#define OTTER_SOURCE_LOCATION() \
    ((otter_source_args) {.file=__FILE__, .func=__func__, .line=__LINE__})

#define OTTER_INITIALISE() \
    otterTraceInitialise()

#define OTTER_FINALISE() \
    otterTraceFinalise()

/**
 * @brief Declares a null task handle in the current scope.
 * 
 */
#define OTTER_DECLARE(task) \
    otter_task_context* task = 0

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
 * @param parent: The handle of the parent of the new task.
 * @param flavour: The user-defined flavour of the new task.
 * @param push_task: Whether to associate the task with the given label.
 * @param format: the format of the label, using subsequent arguments.
 * 
 */
#define OTTER_INIT(task, parent, flavour, push_task, format, ...) \
    task = otterTaskInitialise(parent, flavour, push_task, OTTER_SOURCE_LOCATION(), format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Push the initialised task handle into the set of tasks associated with
 * the label given by the format string and subsequent values.
 * 
 * @warning No check is performed if a given task handle is added to the same
 * label multiple times.
 * 
 */
#define OTTER_PUSH(task, format, ...) \
    otterTaskPushLabel(task, format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Pop a task handle from the set of tasks associated with the label
 * given by the format string and subsequent values. Returns NULL if no tasks
 * are available under the given label.
 * 
 */
#define OTTER_POP(task, format, ...) \
    task = otterTaskPopLabel(format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Borrow a task handle from the set of tasks associated with the label
 * given by the format string and subsequent values. Returns NULL if no tasks
 * are available under the given label. Note that the caller does not own the
 * returned task handle and should not call OTTER_TASK_START or
 * OTTER_TASK_END with it.
 * 
 */
#define OTTER_BORROW(task, format, ...) \
    task = otterTaskBorrowLabel(format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Record the start of a region which represents previously initialised
 * task.
 * 
 * Indicate that the code enclosed by a matching `OTTER_TASK_END()` represents a
 * task which could be scheduled. Note that this does not mean that the enclosed
 * code is actually a task, rather that it could/should be a task after
 * parallelisation.
 * 
 * 
 * ## Usage
 * 
 * - Must be matched by a `OTTER_TASK_END()` call enclosing a region which could/
 *   should be a task.
 * - No synchronisation constraints are recorded by default. To indicate that
 *   a task should be synchronised, see `OTTER_SYNCHRONISE()`.
 * 
 * ## Semantics
 * 
 * Records a `task-switch` event at the current source location which indicates
 * a switch from the encountering task to the new task.
 * 
 * @param task The handle to the task representing the annotated region of code.
 */
#define OTTER_TASK_START(task) \
    task = otterTaskStart(task, OTTER_SOURCE_LOCATION())

/**
 * @brief Counterpart to `OTTER_TASK_START()`, indicating the end of the code 
 * representing the given task.
 * 
 * @param task The completed task.
 *
 * @see `OTTER_TASK_START()`
 */
#define OTTER_TASK_END(task) \
    otterTaskEnd(task, OTTER_SOURCE_LOCATION())

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
 * the encountering task (`children`) or all descendants of the encountering
 * task (`descendants`).
 * 
 */
#define OTTER_SYNCHRONISE(task, mode) \
    otterSynchroniseTasks(task, otter_sync_ ## mode)

#else // define macros as no-op

#define OTTER_INITIALISE()
#define OTTER_FINALISE()
#define OTTER_DECLARE(...)
#define OTTER_INIT(...)
#define OTTER_PUSH(...)
#define OTTER_POP(...)
#define OTTER_TASK_START(...)
#define OTTER_TASK_END(...)
#define OTTER_SYNCHRONISE(...)

#endif

#endif // OTTER_TASK_GRAPH_API_MACRO_H
