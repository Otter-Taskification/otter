#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

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
void otterTaskBeginSingle(void);
void otterTaskEndSingle(void);
void otterLoopBegin(void);
void otterLoopEnd(void);
void otterLoopIterationBegin(void);
void otterLoopIterationEnd(void);
void otterSynchroniseChildTasks(void);
void otterSynchroniseDescendantTasks(void);

#endif // OTTER_SERIAL_H
