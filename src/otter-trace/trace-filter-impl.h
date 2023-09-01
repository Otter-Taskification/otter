#define _GNU_SOURCE // for getline
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "public/debug.h"
#include "public/otter-common.h"

#include "trace-filter.h"

#define MAXKEY 16 // max key length

// bits for rule flags
#define RULE_INIT_BIT (1 << 0)
#define RULE_START_BIT (1 << 1)
#define CHK_RULE_INIT_BIT(k) ((k) & RULE_INIT_BIT)
#define CHK_RULE_START_BIT(k) ((k) & RULE_START_BIT)
#define SET_RULE_INIT_BIT(k) ((k) |= RULE_INIT_BIT)
#define SET_RULE_START_BIT(k) ((k) |= RULE_START_BIT)

typedef enum filter_mode_t {
  mode_invalid,
  mode_include,
  mode_exclude
} filter_mode_t;

// A rule can contain any of {label, init, start}
typedef struct Rule {
  unsigned char flags;
  char *label;
  otter_src_location_t init;
  otter_src_location_t start;
} Rule;
#define RULE_INITIALISER ((Rule){0, NULL, {NULL, NULL, -1}, {NULL, NULL, -1}})

// A vector of rules
typedef struct Rule_v {
  const Rule *rules; // rules array
  const Rule *next;  // pointer to next slot in rules[]
  size_t cap;        // capacity
} Rule_v;
#define RULE_V_INITIALISER ((Rule_v){NULL, NULL, 0})

// A filter contains init rules and/or start rules
typedef struct trace_filter_t {
  Rule_v init;  // rules which apply at "init"
  Rule_v start; // rules which apply at "start"
  bool include;
} trace_filter_t;
#define FILTER_INITIALISER                                                     \
  ((trace_filter_t){RULE_V_INITIALISER, RULE_V_INITIALISER, 0})

// rule vectors
static void rule_v_init(Rule_v *, size_t);
static void rule_v_append_copy(Rule_v *, Rule);

// filters
static trace_filter_t *filter_alloc(void);
static trace_filter_t *filter_initialise(trace_filter_t *, size_t, size_t,
                                         bool);

// parsing parts of rules
static filter_mode_t parse_filter_mode(char *);
static void parse_source_location(otter_src_location_t *, char *);

// print a single rule
static void trace_filter_rule_fwrite(const Rule *, FILE *);
