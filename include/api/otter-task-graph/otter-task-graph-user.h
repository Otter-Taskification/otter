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

/**
 * @brief Need macros for the following:
 * 
 * - call otterPhaseBegin/otterPhaseSwitch/otterPhaseEnd
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

#define OTTER_INVALID_TASK 0

#define OTTER_INITIALISE() \
    otterTraceInitialise()

#define OTTER_FINALISE() \
    otterTraceFinalise()

/**
 * @brief Declares a null task handle in the current scope.
 * 
 */
#define OTTER_DECLARE(task) \
    otter_task_context* task = OTTER_INVALID_TASK

/**
 * @brief Initialise a task handle with the given flavour as a child of parent.
 * If `parent` is NULL, the new task handle has no parent task. If `push_task`
 * is true, the task will be stored internally under a label given by `format`
 * and any subsequent arguments.
 * 
 * @param task: otter_task_context*
 * @param parent: otter_task_context* or NULL
 * @param flavour: int
 * @param push_task: bool
 * @param format: a string literal used to interpret subsequent arguments.
 * 
 */
#define OTTER_INIT(task, parent, flavour, push_task, format, ...) \
    task = otterTaskInitialise_v(parent, flavour, push_task, (otter_source_args) {.file=__FILE__, .func=__func__, .line=__LINE__}, format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Push the initialised task handle into the set of tasks associated with
 * the label given by the format string and subsequent values.
 * 
 * @warning No check is performed if a given task handle is added to the same
 * label multiple times.
 * 
 */
#define OTTER_PUSH(task, format, ...) \
    otterTaskPushLabel_v(task, format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Pop a task handle from the set of tasks associated with the label
 * given by the format string and subsequent values. Returns NULL if no tasks
 * are available under the given label.
 * 
 */
#define OTTER_POP(task, format, ...) \
    task = otterTaskPopLabel_v(format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Borrow a task handle from the set of tasks associated with the label
 * given by the format string and subsequent values. Returns NULL if no tasks
 * are available under the given label. Note that the caller does not own the
 * returned task handle and should not call OTTER_TASK_START or
 * OTTER_TASK_FINISH with it.
 * 
 */
#define OTTER_BORROW(task, format, ...) \
    task = otterTaskBorrowLabel_v(format OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Indicate the start of a region of code representing the given task
 * handle.
 * 
 */
#define OTTER_TASK_START(task) \
    task = otterTaskStart(OTTER_SRC_ARGS(), task)

/**
 * @brief Indicate the end of a region of code representing the given task
 * handle.
 * 
 */
#define OTTER_TASK_FINISH(task) \
    otterTaskEnd(task)

/**
 * @brief Indicate a synchronisation point during the execution of the given
 * task handle. The synchronisation constraint applies to the children or
 * descendants of the given task. Valid values for `mode` are the tokens
 * `children` or `descendants`.
 * 
 */
#define OTTER_SYNCHRONISE(task, mode) \
    otterSynchroniseTasks(task, otter_sync_ ## mode)

#undef OTTER_IMPL_THIRD_ARG
#undef OTTER_IMPL_VA_OPT_AVAIL_I
#undef OTTER_IMPL_VA_OPT_AVAIL
#undef OTTER_IMPL_PASS_ARGS_I
#undef OTTER_IMPL_PASS_ARGS_I
#undef OTTER_IMPL_PASS_ARGS

#else

#define OTTER_INITIALISE()
#define OTTER_FINALISE()
#define OTTER_DECLARE(...)
#define OTTER_INIT(...)
#define OTTER_PUSH(...)
#define OTTER_POP(...)
#define OTTER_TASK_START(...)
#define OTTER_TASK_FINISH(...)
#define OTTER_SYNCHRONISE(...)

#endif

#endif // OTTER_TASK_GRAPH_API_MACRO_H
