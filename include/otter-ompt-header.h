#if !defined(OTTER_OMPT_HEADER_H)
#define OTTER_OMPT_HEADER_H

#if defined(__clang__) && !(defined(__INTEL_COMPILER) || defined(__INTEL_CLANG_COMPILER))
#include <ompt.h>
#else
#include <omp-tools.h>
#endif

#endif // OTTER_OMPT_HEADER_H
