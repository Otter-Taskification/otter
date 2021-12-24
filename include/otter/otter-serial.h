#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
The public API for the Otter serial programme tracing library
*/

// API entrypoints
void otterTraceInitialise(void);
void otterTraceFinalise(void);
void otterParallelBegin(void);
void otterParallelEnd(void);
void otterTaskBegin(void);
void otterTaskEnd(void);
void otterTaskSingleBegin(void);
void otterTaskSingleEnd(void);
void otterLoopBegin(void);
void otterLoopEnd(void);
void otterLoopIterationBegin(void);
void otterLoopIterationEnd(void);
void otterSynchroniseChildTasks(void);
void otterSynchroniseDescendantTasksBegin(void);
void otterSynchroniseDescendantTasksEnd(void);

void otterTraceStart(void);
void otterTraceStop(void);

#ifdef __cplusplus
}
#endif

#endif // OTTER_SERIAL_H
