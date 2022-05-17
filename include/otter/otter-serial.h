#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

/**
 * @todo finalise "simple trace" event source for use in single-threaded app (only with updates to current implementation of otter-serial)
 *   @todo refactor otterParallel[Begin|End] -> otterThreads[Begin|End]
 *   @todo replace otterSynchroniseDescendantTasks[Begin|End] with macro/function barrier to synchronise child/descendant tasks
 * @todo (eventually) make otterParallel[Begin|End] optional (will require re-engineering otter-trace)
 * @todo (eventually) add the concept of "phases" -> probably depends on re-engineering of otter-trace
 * @todo (eventually) need to be able to start/stop tracing at will -> probably depends on re-engineering of otter-trace
 * @todo (long term) generic trace API that accepts diverse event sources from user code
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise Otter overall
 *
 * This has to be the very first macro that you invoke.
 *
 * ## Usage
 *
 * - You have to call this routine early throughput the program
 *   execution. No Otter call should precede this operation.
 * - Ensure that the routine's counterpart otterTraceFinalise()
 *   is called as well as very last operation of your code.
 * - [optional] If you work with a tasking backend which works with a fixed
 *   number of threads which are persistent and use lightweight
 *   tasks (or logical threads), call otterParallelBegin().
 * - [optional] You might want to stop the tracing of events immediately
 *   after this function call via a call to otterTraceStop(). In this case,
 *   you have to manually invoke otterTraceStart() later on.
 *
 * ## Semantics
 *
 * @todo Adam Describe what this is doing.
 *
 * The routine implicitly calls otterTraceStart().
 */
#define otterTraceInitialise()                                                 \
    otterTraceInitialise_i(__FILE__, __func__, __LINE__)


/**
 * Begin a parallel region
 *
 * This is the classic thread-spawn entry point, i.e. when you kick up
 * multiple threads. If you don't have multiple threads or if you don't
 * know when they are created, you don't have to call this routine, i.e.
 * it is an optional marker which is not found in each and every
 * Otter-annotated code.
 *
 * If you use the routine, please ensure that you also use a corresponding
 * otterParallelEnd().
 *
 *
 * ## Usage
 *
 * - Insert otterParallelBegin().
 * - Ensure that it is matched with a corresponding otterParallelEnd().
 *
 *
 * ## Semantics
 *
 * @Adam explanation if necessary
 *
 *
 * ## OpenMP
 *
 * In OpenMP, this routine would be the omp parallel where you issue your
 * threads without distributing work on it already. It would/could also
 * correspond to a teams statement where you issue a whole team - maybe
 * even on another device.
 */
#define otterParallelBegin()                                                   \
    otterParallelBegin_i(__FILE__, __func__, __LINE__)


/**
 * Annotate that a task begins here
 *
 * This marker indicates that a task begins in a certain line. It does not
 * necessarily mean that the code starting after this statement is already a
 * task, but merely indicates that you think there could be task here.
 *
 *
 * ## Usage
 *
 * - Insert the otterTaskBegin() call just where your task is about to start.
 * - Ensure there is a statement otterTaskEnd() where the task terminates.
 * - Please note that the task termination does not mean any synchronisation.
 *   You have to add any required synchronisation points (dependencies)
 *   manually via otterSynchroniseChildTasks() or
 *   otterSynchroniseDescendentTasks().
 *
 *
 * ## Semantics
 *
 * The end of a task does not imply in any way that there should be any
 * synchronisation. If you create a task, you usually insert some waits
 * somewhere.
 */
#define otterTaskBegin()                                                       \
    otterTaskBegin_i(__FILE__, __func__, __LINE__)


/**
 * Indicate that a loop begins here
 *
 * This should be used if and only if you want to mark a loop which could run
 * in parallel or is already parallelised.
 *
 * ## Usage
 *
 * - Add the otterLoopBegin() call just before the loop.
 * - Ensure there's a matching otterLoopEnd().
 * - Optional: Insert pairs of otterLoopIterationBegin() and
 *   otterLoopIterationEnd(). This will allow Otter to measure how expensive
 *   individual loop iterations are and
 */
#define otterLoopBegin()                                                       \
    otterLoopBegin_i(__FILE__, __func__, __LINE__)


/**
 * @see otterLoopBegin()
 */
#define otterLoopIterationBegin()                                              \
    otterLoopIterationBegin_i(__FILE__, __func__, __LINE__)


/**
 * @todo Adam. I have no clue what this means.
 */
#define otterTaskSingleBegin()                                                 \
    otterTaskSingleBegin_i(__FILE__, __func__, __LINE__)


/**
 * Add synchronisation barrier waiting for children
 *
 * This operation indicates that you have to wait for direct children. It does
 * not mean that you wait for the children of children.
 */
#define otterSynchroniseChildTasks()                                           \
    otterSynchroniseChildTasks_i(__FILE__, __func__, __LINE__)


/**
 * @todo Adam I'm not sure this should be mapped onto a begin() and end() thing.
 *  I think that's too close to Peano's internal semantics and/or OpenMP working.
 *  Let's just provide otterSynchroniseDescendantTasks().
 */
#define otterSynchroniseDescendantTasksBegin()                                 \
    otterSynchroniseDescendantTasksBegin_i(__FILE__, __func__, __LINE__)


/**
 * @todo Adam. That's a macro I'd really like to have
 */
#define otterSynchroniseDescendantTasks()


/**
 * Public C API for the Otter's tracing
 *
 * Each of these functions has a _i postscript which distinguishes it from the
 * macro. We recommend that you start with the macros instead. Once the macros
 * give you a reasonable first analysis of your code, it makes sense to switch
 * to these _i functions which allow you to provide more context and details
 * to Otter.
 *
 * @see otterTraceInitialise()
 */
void otterTraceInitialise_i(const char*, const char*, const int);


/**
 * Counterpart to otterTraceInitialise().
 *
 * @see otterTraceInitialise()
 */
void otterTraceFinalise(void);


/**
 * Public C API for the Otter's tracing
 *
 * Each of these functions has a _i postscript which distinguishes it from the
 * macro. We recommend that you start with the macros instead. Once the macros
 * give you a reasonable first analysis of your code, it makes sense to switch
 * to these _i functions which allow you to provide more context and details
 * to Otter.
 *
 * @todo Adam I think this name is too close to OpenMP. We should name it ThreadBegin() in my opinion.
 *
 * @see otterParallelBegin()
 */
void otterParallelBegin_i(const char*, const char*, const int);


/**
 * Counterpart to otterParallelBegin().
 *
 * @see otterParallelBegin()
 */
void otterParallelEnd(void);


/**
 * Public C API for the Otter's tracing
 *
 * Each of these functions has a _i postscript which distinguishes it from the
 * macro. We recommend that you start with the macros instead. Once the macros
 * give you a reasonable first analysis of your code, it makes sense to switch
 * to these _i functions which allow you to provide more context and details
 * to Otter.
 *
 * @see otterTaskBegin()
 */
void otterTaskBegin_i(const char*, const char*, const int);


/**
 * Counterpart to otterTaskBegin().
 *
 * @see otterTaskBegin()
 */
void otterTaskEnd(void);


void otterTaskSingleBegin_i(const char*, const char*, const int);
void otterTaskSingleEnd(void);
void otterLoopBegin_i(const char*, const char*, const int);
void otterLoopEnd(void);
void otterLoopIterationBegin_i(const char*, const char*, const int);
void otterLoopIterationEnd(void);
void otterSynchroniseChildTasks_i(const char*, const char*, const int);
void otterSynchroniseDescendantTasksBegin_i(const char*, const char*, const int);
void otterSynchroniseDescendantTasksEnd(void);


/**
 * Start tracing
 *
 * The tracing is automatically started by otterTraceInitialise() but you can
 * stop it any time and then resume it via otterTraceStart().
 *
 * @see otterTraceStop()
 * @see otterTraceInitialise()
 */
void otterTraceStart(void);


/**
 * Suspend tracing
 *
 * To re-activate the tracing, you have to call otterTraceStart().
 */
void otterTraceStop(void);


/**
 * Start a new algorithmic phase
 *
 * By default, all trace events fall into the same global rubric. However, some
 * codes run through particular phases and will want to study these phases
 * independently. With the present routine you mark the begin of such a phase.
 * Each phase has to be given a unique name.
 *
 * @todo Adam can we provide this feature?
 *
 * @see otterPhaseEnd()
 * @see otterPhaseSwitch()
 */
void otterPhaseBegin( const char* name );


/**
 * @see otterPhaseBegin()
 * @see otterPhaseSwitch()
 */
void otterPhaseEnd();


/**
 * Close last phase and switch to other one
 */
void otterPhaseSwitch( const char* name );

#ifdef __cplusplus
}
#endif

#endif // OTTER_SERIAL_H
