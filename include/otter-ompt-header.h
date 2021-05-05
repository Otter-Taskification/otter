#if !defined(OTTER_OMPT_HEADER_H)
#define OTTER_OMPT_HEADER_H

#if defined(__INTEL_COMPILER)
#include <omp-tools.h>
#else
#include <ompt.h>
#endif

#endif // OTTER_OMPT_HEADER_H
