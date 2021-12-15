#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
The public API for the Otter serial programme tracing library
*/

// API entrypoints
void otterTraceBegin(void);
void otterTraceEnd(void);
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

#ifdef __cplusplus
}
#endif

#endif // OTTER_SERIAL_H
