#if !defined(OTTER_SERIAL_H)
#define OTTER_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define otterTraceInitialise()                                                 \
    otterTraceInitialise_i(__FILE__, __func__, __LINE__)
#define otterParallelBegin()                                                   \
    otterParallelBegin_i(__FILE__, __func__, __LINE__)
#define otterTaskBegin()                                                       \
    otterTaskBegin_i(__FILE__, __func__, __LINE__)
#define otterTaskSingleBegin()                                                 \
    otterTaskSingleBegin_i(__FILE__, __func__, __LINE__)
#define otterLoopBegin()                                                       \
    otterLoopBegin_i(__FILE__, __func__, __LINE__)
#define otterLoopIterationBegin()                                              \
    otterLoopIterationBegin_i(__FILE__, __func__, __LINE__)
#define otterSynchroniseChildTasks()                                           \
    otterSynchroniseChildTasks_i(__FILE__, __func__, __LINE__)
#define otterSynchroniseDescendantTasksBegin()                                 \
    otterSynchroniseDescendantTasksBegin_i(__FILE__, __func__, __LINE__)

/*
The public API for the Otter serial programme tracing library
*/

// API entrypoints
void otterTraceInitialise_i(const char*, const char*, const int);
void otterTraceFinalise(void);
void otterParallelBegin_i(const char*, const char*, const int);
void otterParallelEnd(void);
void otterTaskBegin_i(const char*, const char*, const int);
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

void otterTraceStart(void);
void otterTraceStop(void);

#ifdef __cplusplus
}
#endif

#endif // OTTER_SERIAL_H
