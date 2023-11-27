/**
 * @file otter-task-graph-user.h
 * @author Adam Tuft
 * @brief Provides macros for accessing the Otter task-graph API for the purpose
 * of annotating user code.
 * @version 0.1
 * @date 2023-05-15
 *
 * @copyright Copyright (c) 2023 Adam Tuft. All rights reserved.
 *
 */

#pragma once

#if defined(OTTER_TASK_GRAPH_DISABLE_USER)

#define OTTER_INITIALISE()
#define OTTER_FINALISE()
#define OTTER_DECLARE_HANDLE(...)
#define OTTER_INIT_TASK(...)
#define OTTER_DEFINE_TASK(...)
#define OTTER_POOL_ADD(...)
#define OTTER_POOL_POP(...)
#define OTTER_POOL_DECL_POP(...)
#define OTTER_POOL_BORROW(...)
#define OTTER_POOL_DECL_BORROW(...)
#define OTTER_TASK_START(...)
#define OTTER_TASK_END(...)
#define OTTER_TASK_WAIT_FOR(...)
#define OTTER_TASK_WAIT_START(...)
#define OTTER_TASK_WAIT_END(...)
#define OTTER_PHASE_BEGIN(...)
#define OTTER_PHASE_END(...)
#define OTTER_PHASE_SWITCH(...)

#else

#define OTTER_USE_PRIVATE_HEADER
#include "otter-task-graph.h"
#undef OTTER_USE_PRIVATE_HEADER

// @cond DOXYGEN_IGNORE

/* detect whether __VA_OPT__ supported
   credit: https://stackoverflow.com/a/48045656
*/
#define OTTER_IMPL_THIRD_ARG(a, b, c, ...) c
#define OTTER_IMPL_VA_OPT_AVAIL_I(...)                                         \
  OTTER_IMPL_THIRD_ARG(__VA_OPT__(, ), 1, 0, )
#define OTTER_IMPL_VA_OPT_AVAIL OTTER_IMPL_VA_OPT_AVAIL_I(?)

/* my addition to make variadic macros agnostic of __VA_OPT__ support */
#if OTTER_IMPL_VA_OPT_AVAIL
#define OTTER_IMPL_PASS_ARGS_I(...) __VA_OPT__(, ) __VA_ARGS__
#else
#define OTTER_IMPL_PASS_ARGS_I(...) , ##__VA_ARGS__
#endif
#define OTTER_IMPL_PASS_ARGS(...) OTTER_IMPL_PASS_ARGS_I(__VA_ARGS__)

#define OTTER_SOURCE_LOCATION()                                                \
  ((otter_source_args){.file = __FILE__, .func = __func__, .line = __LINE__})

// @endcond

/**
 * @brief Used to express the absence of a task e.g. when creating a task with
 * no parent.
 *
 */
#ifdef __cplusplus
#define OTTER_NULL_TASK nullptr
#else
#define OTTER_NULL_TASK ((void *)0)
#endif

/**
 * @brief Start Otter. Must be invoked before any other Otter function or macro.
 *
 */
#define OTTER_INITIALISE() otterTraceInitialise(OTTER_SOURCE_LOCATION())

/**
 * @brief Shut down Otter. Must be called just before program termination.
 *
 */
#define OTTER_FINALISE() otterTraceFinalise(OTTER_SOURCE_LOCATION())

/**
 * @brief Declares a null task handle in the current scope.
 *
 */
#define OTTER_DECLARE_HANDLE(task) otter_task_context *task = OTTER_NULL_TASK

/**
 * @brief Initialise a new task instance using the given handle.
 *
 * If \p parent is a valid task handle, the new task is a child of this parent.
 *
 * If \p parent is `OTTER_NULL_TASK`, the new task has no parent task.
 *
 * If `add_to_pool` is `otter_add_to_pool`, the task will be added to the task
 * pool with the given label.
 *
 * @note Records a `task-create` event in the trace. The option to override this
 * is currently hardcoded out, but is planned to be supported.
 *
 * @param task: The handle for the new task.
 * @param parent: The handle of the parent task, or #OTTER_NULL_TASK if there
 * is no parent task.
 * @param add_to_pool: Whether to add the task to the task pool with the given
 * label. Must be either otter_add_to_pool or otter_no_add_to_pool.
 * @param label: A `printf`-like format string for the task's label
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_INIT_TASK(task, parent, add_to_pool, label, ...)                 \
  task = otterTaskInitialise(parent, -1, add_to_pool, true,                    \
                             OTTER_SOURCE_LOCATION(),                          \
                             label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Declare and initialise a new task handle in the current scope.
 *
 * Equivalent to OTTER_DECLARE_HANDLE() followed by OTTER_INIT_TASK().
 *
 * If \p parent is a valid task handle, the new task is a child of this parent.
 *
 * If \p parent is #OTTER_NULL_TASK, the new task has no parent task.
 *
 * If `add_to_pool` is `otter_add_to_pool`, the task will be added to the task
 * pool with the given label.
 *
 * @note Records a `task-create` event in the trace. The option to override this
 * is currently hardcoded out, but is planned to be supported.
 *
 * @param task: The handle for the new task.
 * @param parent: The handle of the parent task, or #OTTER_NULL_TASK if there
 * is no parent task.
 * @param add_to_pool: Whether to add the task to the task pool with the given
 * label. Must be either otter_add_to_pool or otter_no_add_to_pool.
 * @param label: A `printf`-like format string for the task's label
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_DEFINE_TASK(task, parent, add_to_pool, label, ...)               \
  OTTER_DECLARE_HANDLE(task);                                                  \
  OTTER_INIT_TASK(task, parent, add_to_pool,                                   \
                  label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Add a task handle to the task pool with the given label.
 *
 * @warning It is an error to add the same task instance to the task pool
 * multiple times.
 *
 * @param task: The task to add to the task pool.
 * @param label: A `printf`-like format string for the task's label.
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_POOL_ADD(task, label, ...)                                       \
  otterTaskPushLabel(task, label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Remove a task from the task pool with the given label. \p task is
 * `OTTER_NULL_TASK` if no tasks are available.
 *
 * @note The caller owns the returned task and may call `OTTER_TASK_START` or
 * `OTTER_TASK_END` on it.
 *
 * @param task: The handle for the retrieved task.
 * @param label: A `printf`-like format string for the task's label.
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_POOL_POP(task, label, ...)                                       \
  task = otterTaskPopLabel(label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Borrow a task from the task pool with the given label. \p task is
 * `OTTER_NULL_TASK` if no tasks are available. The borrowed task remains in the
 * pool to be borrowed (or possibly removed) elsewhere.
 *
 * @note The caller does not own the borrowed task and must not call
 * `OTTER_TASK_START` or `OTTER_TASK_END` on it. The only valid operation on a
 * borrowed task is to pass it as the parent task to `OTTER_INIT_TASK`.
 *
 * @note The user must take care that a borrowed task is not also passed to
 * `OTTER_TASK_END` while it is being borrowed.
 *
 * @param task: The handle for the borrowed task.
 * @param label: A `printf`-like format string for the task's label.
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_POOL_BORROW(task, label, ...)                                    \
  task = otterTaskBorrowLabel(label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Declare a handle in the current scope, assigning a task removed from
 * the task pool with the given label. \p task is `OTTER_NULL_TASK` if no tasks
 * are available.
 *
 * @note The caller owns the returned task and may call `OTTER_TASK_START` or
 * `OTTER_TASK_END` on it.
 *
 * @param task: The handle for the retrieved task.
 * @param label: A `printf`-like format string for the task's label.
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_POOL_DECL_POP(task, label, ...)                                  \
  OTTER_DECLARE_HANDLE(task);                                                  \
  OTTER_POOL_POP(task, label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Declare a handle in the current scope, assigning a task borrowed from
 * the task pool with the given label. \p task is `OTTER_NULL_TASK` if no tasks
 * are available. The borrowed task remains in the pool to be borrowed (or
 * possibly removed) elsewhere.
 *
 * @note The caller does not own the borrowed task and must not call
 * `OTTER_TASK_START` or `OTTER_TASK_END` on it. The only valid operation on a
 * borrowed task is to pass it as the parent task to `OTTER_INIT_TASK`.
 *
 * @note The user must take care that a borrowed task is not also passed to
 * `OTTER_TASK_END` while it is being borrowed.
 *
 * @param task: The handle for the borrowed task.
 * @param label: A `printf`-like format string for the task's label.
 * @param ...: Variadic arguments for use with \p label.
 *
 */
#define OTTER_POOL_DECL_BORROW(task, label, ...)                               \
  OTTER_DECLARE_HANDLE(task);                                                  \
  OTTER_POOL_BORROW(task, label OTTER_IMPL_PASS_ARGS(__VA_ARGS__))

/**
 * @brief Record the start of the code represented by the given task handle.
 *
 * Indicate that the code enclosed by a matching `OTTER_TASK_END()` is
 * considered a task which could be scheduled. Note that this does not mean that
 * the enclosed code is actually a task, rather that it could/should be a task
 * after parallelisation.
 *
 * @warning The caller must own the given task instance i.e. it must not be
 * borrowed.
 *
 * @note Must be matched by a `OTTER_TASK_END()` call in the same scope.
 *
 * @note No synchronisation constraints are recorded by default. To indicate
 * that a task should be synchronised, see `OTTER_SYNCHRONISE()`.
 *
 * ## Usage
 *
 * ### OpenMP
 *
 * When annotating OpenMP tasks, `OTTER_TASK_START` and `OTTER_TASK_END` must
 * appear **inside** the body of the task, like so:
 *
 *     #pragma omp task
 *     {
 *      OTTER_TASK_START(...);
 *      // ...
 *      OTTER_TASK_END(...);
 *     }
 *
 * This allows the start and end time of the task to be recorded when the task
 * is actually executed.
 *
 * @param task The task representing the annotated region of code.
 *
 * @see #OTTER_TASK_END
 */
#define OTTER_TASK_START(task)                                                 \
  task = otterTaskStart(task, OTTER_SOURCE_LOCATION())

/**
 * @brief Counterpart to `OTTER_TASK_START()`, indicating the end of the code
 * represented by the given task handle.
 *
 * @warning The caller must own the given task instance i.e. it must not be
 * borrowed.
 *
 * @param task: The task representing the annotated region of code.
 *
 * @see #OTTER_TASK_START
 */
#define OTTER_TASK_END(task) otterTaskEnd(task, OTTER_SOURCE_LOCATION())

/**
 * @brief Records a barrier where the given task must wait until all prior child
 * or descendant tasks are complete.
 *
 * @note This constraint is not enforced but is simply recorded in the trace.
 *
 * @param task: The task which encounters the barrier.
 * @param mode: Whether this barrier applies to the children of the encountering
 * task (`children`) or all descendants of the encountering task
 * (`descendants`).
 *
 */
#define OTTER_TASK_WAIT_FOR(task, mode)                                        \
  otterSynchroniseTasks(task, otter_sync_##mode, otter_endpoint_discrete)

/**
 * @brief Record the start of a region where the task waits for children or
 * descendants to complete. Inside this region the task is considered by Otter
 * to be suspended, so time spent in this region is not attributed to the
 * duration of this task.
 *
 * @note Counterpart to #OTTER_TASK_WAIT_END
 *
 * ## Usage
 *
 * ### OpenMP
 *
 * This annotation may be used to decorate a `taskwait` barrier like so:
 *
 * OTTER_TASK_WAIT_START(task, children)
 * #pragma omp taskwait
 * OTTER_TASK_WAIT_END(task, children)
 *
 * This will then record the time at which the task enters/leaves the task
 * scheduling point at the barrier.
 *
 */
#define OTTER_TASK_WAIT_START(task, mode)                                      \
  otterSynchroniseTasks(task, otter_sync_##mode, otter_endpoint_enter)

/**
 * @brief Record the end of a region where the task waits for children or
 * descendants to complete.
 *
 * @note Counterpart to #OTTER_TASK_WAIT_START
 *
 */
#define OTTER_TASK_WAIT_END(task, mode)                                        \
  otterSynchroniseTasks(task, otter_sync_##mode, otter_endpoint_leave)

/**
 * @brief Start a new algorithmic phase.
 *
 * By default, all trace events fall into the same global phase. However, some
 * codes run through particular phases and will want to study these phases
 * independently. With the present routine you mark the begin of such a phase.
 * Each phase has to be given a unique name.
 *
 *
 * ## Usage
 *
 * - Must be matched by a corresponding `OTTER_PHASE_END()` or
 * `OTTER_PHASE_SWITCH()`.
 *
 *
 * ## Semantics
 *
 * Creates a meta-region to nest all other regions encountered within it.
 *
 *
 * @param name A unique string identifying this phase.
 *
 * @see `OTTER_PHASE_END()`
 * @see `OTTER_PHASE_SWITCH()`
 *
 */
#define OTTER_PHASE_BEGIN(name) otterPhaseBegin((name), OTTER_SOURCE_LOCATION())

/**
 * @brief End the present algorithmic phase.
 *
 * Indicates the end of the present algorithmic phase and return to the
 * default global phase.
 *
 * @see `OTTER_PHASE_BEGIN()`
 * @see `OTTER_PHASE_SWITCH()`
 *
 */
#define OTTER_PHASE_END() otterPhaseEnd(OTTER_SOURCE_LOCATION())

/**
 * @brief End the present algorithmic phase and immediately switch to another.
 *
 * Indicates the end of the present algorithmic phase and the immediate start
 * of another.
 *
 * Equivalent to:
 *
 *     OTTER_PHASE_END();
 *     OTTER_PHASE_BEGIN(...);
 *
 * @param name The name of the next phase to begin.
 *
 * @see `OTTER_PHASE_BEGIN()`
 * @see `OTTER_PHASE_END()`
 *
 */
#define OTTER_PHASE_SWITCH(name)                                               \
  otterPhaseSwitch((name), OTTER_SOURCE_LOCATION())

#endif
