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
 * TODO:
 * 
 *  - update documentation re. registering tasks against labels
 */

#if !defined(OTTER_TASK_GRAPH_API_MACRO_H)
#define OTTER_TASK_GRAPH_API_MACRO_H

#if defined(OTTER_TASK_GRAPH_ENABLE_USER)
#include "otter-task-graph.h"

/* detect whether __VA_OPT__ supported 
   credit: https://stackoverflow.com/a/48045656
*/
#define THIRD_ARG(a,b,c,...) c
#define VA_OPT_AVAIL_I(...) THIRD_ARG(__VA_OPT__(,),1,0,)
#define VA_OPT_AVAIL VA_OPT_AVAIL_I(?)

/* my addition to make variadic macros agnostic of __VA_OPT__ support */
#if VA_OPT_AVAIL
#define PASS_ARGS_I(...) __VA_OPT__(,) __VA_ARGS__
#else
#define PASS_ARGS_I(...) , ##__VA_ARGS__
#endif
#define PASS_ARGS(...) PASS_ARGS_I(__VA_ARGS__)

#define OTTER_INVALID_TASK 0

#define OTTER_INITIALISE() \
    otterTraceInitialise()

#define OTTER_FINALISE() \
    otterTraceFinalise()

/**
 * @brief Declares a null task handle
 * 
 */
#define OTTER_TASK_GRAPH_DECLARE_TASK(task) \
    otter_task_context* task = OTTER_INVALID_TASK

/**
 * @brief Initialise and optionally register a task handle with a given label
 * and flavour. If `parent` is non-null, the initialised task is considered a
 * child of the given parent. If `should_register` is true, the task will be 
 * registered against the given label
 * 
 */
// TODO: accept variadic label, rename should_register -> push_task
#define OTTER_TASK_GRAPH_INIT_TASK(task, label, flavour, parent, should_register) \
    task = otterTaskInitialise(label, flavour, parent, should_register)

/**
 * @brief Register the initialised task handle with the label given by the 
 * format string and any subsequent values. Silently overwrites the registered
 * handle if the label was previously registered.
 * 
 */
// TODO: rename to OTTER_TASK_GRAPH_POP_TASK
#define OTTER_TASK_GRAPH_REGISTER_TASK(task, format, ...) \
    otterTaskRegisterLabel_v(task, format PASS_ARGS(__VA_ARGS__))

/**
 * @brief Pop and assign the task handle previously registered with the label 
 * given by the format string and any subsequent values, or null if no task was
 * registered.
 * 
 */
#define OTTER_TASK_GRAPH_POP_TASK(task, format, ...) \
    task = otterTaskPopLabel_v(format PASS_ARGS(__VA_ARGS__))

/**
 * @brief Indicate the start of a region of code representing the given task
 * handle.
 * 
 */
#define OTTER_TASK_GRAPH_START_TASK(task, flavour) \
    task = otterTaskStart(OTTER_SRC_ARGS(), task, flavour)

/**
 * @brief Indicate the end of a region of code representing the given task
 * handle.
 * 
 */
#define OTTER_TASK_GRAPH_FINISH_TASK(task) \
    otterTaskEnd(task)

/**
 * @brief Indicate a synchronisation point during the execution of the given
 * task handle. The synchronisation constraint applies to the children or
 * descendants of the given task. Valid values for `mode` are the tokens
 * `children` or `descendants`.
 * 
 */
#define OTTER_TASK_GRAPH_SYNCHRONISE_TASKS(task, mode) \
    otterSynchroniseTasks(task, otter_sync_ ## mode)

#else

#define OTTER_INITIALISE()
#define OTTER_FINALISE()
#define OTTER_TASK_GRAPH_DECLARE_TASK(...)
#define OTTER_TASK_GRAPH_INIT_TASK(...)
#define OTTER_TASK_GRAPH_REGISTER_TASK(...)
#define OTTER_TASK_GRAPH_GET_TASK(...)
#define OTTER_TASK_GRAPH_POP_TASK(...)
#define OTTER_TASK_GRAPH_START_TASK(...)
#define OTTER_TASK_GRAPH_FINISH_TASK(...)
#define OTTER_TASK_GRAPH_SYNCHRONISE_TASKS(...)

#endif

#endif // OTTER_TASK_GRAPH_API_MACRO_H
