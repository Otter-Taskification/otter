#if !defined(OTTER_THREADS_H)
#define OTTER_THREADS_H

// TODO: does GCC also use _Thread_local?

#if defined(__STDC_NO_THREADS__)
#warning "no threads.h"
#define thread_local _Thread_local
#else
#include <threads.h>
#endif

#endif // OTTER_THREADS_H
