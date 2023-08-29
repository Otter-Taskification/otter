#include <stdio.h>

typedef struct trace_filter_t trace_filter_t;

int trace_filter_load(trace_filter_t **, FILE *);
void trace_filter_fwrite(const trace_filter_t *, FILE *);
