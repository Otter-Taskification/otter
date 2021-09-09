#if !defined(OTTER_OMPT_HEADER_H)
#define OTTER_OMPT_HEADER_H

// #if defined(__clang__) && !(defined(__INTEL_COMPILER) || defined(__INTEL_CLANG_COMPILER))
//     #include <ompt.h>
//     #if __clang_major__ >= 12
//     #define USE_OMPT_MASKED
//     #endif
// #else
//     #define USE_OMPT_MASKED
//     #include <omp-tools.h>
// #endif

// #if defined(HAVE_OMP_TOOLS_H)
//     #define USE_OMPT_MASKED
//     #include <omp-tools.h>
// #elif defined(HAVE_OMPT_H)
//     #if __clang_major__ >= 12
//         #define USE_OMPT_MASKED
//     #endif
//     #include <ompt.h>
// #else
//     #error missing OMPT header
// #endif

#include <omp-tools.h>

#endif // OTTER_OMPT_HEADER_H
