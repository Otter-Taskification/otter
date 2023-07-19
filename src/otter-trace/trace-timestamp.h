#include <stdint.h>
#include <time.h>

static uint64_t get_timestamp(void) {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return time.tv_sec * (uint64_t)1000000000 + time.tv_nsec;
}
