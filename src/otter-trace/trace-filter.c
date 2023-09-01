/**
 * @file trace-filter.c
 * @author Adam Tuft
 * @brief Implements Otter's filtering mechanism
 * @version 0.1
 * @date 2023-08-29
 *
 * @copyright Copyright (c) 2023 Adam Tuft
 *
 */
#include "trace-filter-impl.h"

int trace_filter_load(trace_filter_t **filt, string_registry *strings,
                      FILE *filter_file) {

  if ((filter_file == NULL) || (filt == NULL)) {
    return -1;
  }

  trace_filter_t *F = filter_init(NULL, 10, 10, false);
  *filt = F;
  Rule rule = RULE_INITIALISER;

  LOG_DEBUG("parsing filter file");

  bool mode_set = false;
  char *line = NULL, *key = NULL, *value = NULL;
  size_t bufsz = 0;
  ssize_t chars_read = 0;
  int rules_parsed = 0;

  while (true) {

    // want to detect whether chars_read == -1 means error or EOF
    errno = 0;
    chars_read = getline(&line, &bufsz, filter_file);
    LOG_DEBUG_IF(false, "bufsz=%lu, chars_read=%ld", bufsz, chars_read);

    if ((chars_read == -1) && (errno != 0)) {

      // error
      LOG_ERROR("failed to read line: %s", strerror(errno));
      break;

    } else if ((chars_read == 1 /* empty line */) ||
               (chars_read == -1 /* EOF */)) {

      // rule is finished
      if (!(rule.flags == 0 && rule.label == OTTER_STRING_UNDEFINED)) {
        Rule_v *coll = CHK_RULE_START_BIT(rule.flags) ? &F->start : &F->init;
        rule_v_append_copy(coll, rule);
        rules_parsed++;
      }
      rule = RULE_INITIALISER;

      if (chars_read == -1) { // EOF
        LOG_DEBUG("parsed %lu rules", rules_parsed);
        break;
      }

    } else {

      // process line to get an item (key-value pair) and add to current rule

      if (line[0] == '#') {
        continue; // skip comment lines
      }

      // replace newline
      if (line[chars_read - 1] == '\n') {
        line[chars_read - 1] = '\0';
      }

      // get key
      int scan_result = 0;
      int consumed = 0;
      if ((scan_result = sscanf(line, "%ms %n", &key, &consumed)) == EOF) {
        LOG_ERROR("invalid line: \"%s\"", line);
        return -1;
      }

      // parse the rest of the line
      value = &line[consumed];
      size_t vlen = strlen(value);

      if (strncmp(key, "mode", MAXKEY) == 0) {

        // This line sets the mode, so detect mode and continue to next line

        if (mode_set) {
          LOG_ERROR("mode may only be specified once.");
          return -1;
        }
        mode_set = true;

        filter_mode_t mode = parse_filter_mode(value);
        if (mode == mode_include) {
          F->include = true;
        } else if (mode == mode_exclude) {
          F->include = false;
        } else {
          LOG_ERROR("invalid mode: %s", value);
          return -1;
        }

        LOG_DEBUG("mode: %s", F->include ? "include" : "exclude");
        free(key);
        continue;
      }

      // set the key for this item
      if (strncmp(key, "label", MAXKEY) == 0) {

        rule.label = string_registry_insert(strings, value);
        LOG_DEBUG("  got label: \"%s\" (sref=%lu)", value, rule.label);

      } else if (strncmp(key, "init", MAXKEY) == 0) {

        otter_src_location_t init;
        parse_source_location(&init, value);
        rule.init =
            get_source_location_ref(strings, init.file, init.func, init.line);
        SET_RULE_INIT_BIT(rule.flags);

        LOG_DEBUG("  got init location:");
        LOG_DEBUG("    file: %s (sref=%lu)", init.file, rule.init.file);
        LOG_DEBUG("    func: %s (sref=%lu)", init.func, rule.init.func);
        LOG_DEBUG("    line: %d", rule.init.line);

      } else if (strncmp(key, "start", MAXKEY) == 0) {

        otter_src_location_t start;
        parse_source_location(&start, value);
        rule.start = get_source_location_ref(strings, start.file, start.func,
                                             start.line);
        SET_RULE_START_BIT(rule.flags);

        LOG_DEBUG("  got start location:");
        LOG_DEBUG("    file: %s (sref=%lu)", start.file, rule.start.file);
        LOG_DEBUG("    func: %s (sref=%lu)", start.func, rule.start.func);
        LOG_DEBUG("    line: %d", rule.start.line);

      } else {
        LOG_ERROR(
            "invalid key: %s (must be one of \"label\", \"init\", \"start\")",
            key);
        return -1;
      }

      free(key);
    }

  } // end while loop

  free(line);

  return 0;
}

// Rule_v

static Rule_v *rule_v_alloc(void) { return malloc(sizeof(Rule_v)); }

static void rule_v_init(Rule_v *coll, size_t sz) {
  coll->rules = malloc(sz * sizeof(coll->rules[0]));
  coll->next = coll->rules;
  coll->cap = sz;
}

static void rule_v_append_copy(Rule_v *coll, Rule rule) {
  // is there space?
  LOG_DEBUG("rules=%p, next=%p, cap=%lu, elem=%lu", coll->rules, coll->next,
            coll->cap, coll->next - coll->rules);
  if (coll->next >= (coll->rules + coll->cap)) {
    LOG_DEBUG("realloc!");
    size_t elem = coll->next - coll->rules;
    coll->cap = coll->cap + 10;
    coll->rules =
        realloc((void *)coll->rules, sizeof(coll->rules[0]) * coll->cap);
    coll->next = coll->rules + elem;
    LOG_DEBUG("rules=%p, next=%p, cap=%lu", coll->rules, coll->next, coll->cap);
  }
  LOG_DEBUG("copy %lu bytes to %p", sizeof(rule), coll->next);
  memcpy((void *)coll->next, &rule, sizeof(rule));
  coll->next++;
  LOG_DEBUG("rules=%p, next=%p, cap=%lu, elem=%lu", coll->rules, coll->next,
            coll->cap, coll->next - coll->rules);
}

// trace_filter_t

static trace_filter_t *filter_alloc(void) {
  return malloc(sizeof(trace_filter_t));
}

static trace_filter_t *filter_init(trace_filter_t *f, size_t ninit,
                                   size_t nstart, bool include) {
  if (f == NULL) {
    f = filter_alloc();
  }
  *f = FILTER_INITIALISER;
  rule_v_init(&f->init, ninit);
  rule_v_init(&f->start, nstart);
  f->include = include;
  return f;
}

// parse things

static inline filter_mode_t parse_filter_mode(char *value) {
  if (strcmp(value, "exclude") == 0) {
    return mode_exclude;
  } else if (strcmp(value, "include") == 0) {
    return mode_include;
  } else {
    return mode_invalid;
  }
}

static inline void parse_source_location(otter_src_location_t *src,
                                         char *value) {

  size_t vlen = strlen(value);

  char sep = ':';
  char *sep_p = NULL;

  if ((sep_p = strrchr(value, sep)) == NULL) {

    // sep not found, value is <file>
    src->file = memcpy(calloc(1 + vlen, sizeof(*src->file)), value, vlen);

  } else {

    char *sub = sep_p + 1;

    // sep is present, value is <file>:<line> or <file>:<func>

    // copy up to sep_p for <file>
    src->file = memcpy(calloc(1 + sep_p - value, sizeof(*src->file)), value,
                       sep_p - value);

    src->line = (int)strtol(sub, NULL, 10);
    if ((src->line) != 0) {
      src->func = NULL;
    } else {
      // assume <func>
      size_t func_name_len = strlen(sep_p + 1); // ...:<func>
      src->func = memcpy(calloc(1 + func_name_len, sizeof(*src->func)),
                         sep_p + 1, func_name_len);
    }
  }
}
