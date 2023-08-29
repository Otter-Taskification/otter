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
#define _GNU_SOURCE // for getline
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "public/debug.h"
#include "public/otter-common.h"
#include "public/types/queue.h"

#include "trace-filter.h"

#define MAXLINE 256
#define MAXKEY 16

typedef enum filter_key_t {
  key_label,
  key_init,
  key_start,
  key_end,
} filter_key_t;

// The possible values in a rule item
typedef union filter_rule_value_t {
  const char *text;
  const otter_src_location_t src;
} filter_rule_value_t;

// A rule item is a key-value pair
typedef struct filter_rule_item_t {
  const filter_key_t key;
  const int flags;
  const filter_rule_value_t value;
} filter_rule_item_t;

// A rule is an array of rule items
typedef struct filter_rule_t {
  // A const pointer to an array of const rule items
  const filter_rule_item_t *const items;
  const size_t num_items;
} filter_rule_t;

// An array of pointers to rules
struct trace_filter_t {
  // a const pointer to an array of const pointers to const rules
  const filter_rule_t *const *const rules;
  const size_t num_rules;
  const bool include;
};

int trace_filter_load(trace_filter_t **filt, FILE *filter_file) {

  if ((filter_file == NULL) || (filt == NULL)) {
    return -1;
  }

  *filt = malloc(sizeof(**filt));
  trace_filter_t *filter = *filt;

  LOG_DEBUG("parsing filter file");

  bool mode_set = false;
  char *line = NULL, *key = NULL, *value = NULL, *dest = NULL;
  size_t bufsz = 0;
  ssize_t chars_read = 0;
  otter_queue_t *rule_items = NULL;
  otter_queue_t *rules = queue_create();
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

    } else if ((chars_read == 1) || (chars_read == -1)) {

      // empty line or EOF, current rule finished
      if (rule_items != NULL) {

        rules_parsed++;

        size_t num_items = queue_length(rule_items);
        LOG_DEBUG("finished rule %d with %lu items", rules_parsed, num_items);

        // move rule items into an array for better memory locality
        filter_rule_item_t *rule_items_v = NULL;
        size_t rule_item_sz = sizeof(*rule_items_v);
        rule_items_v = malloc(num_items * rule_item_sz);

        // move each item into the array
        filter_rule_item_t *temp = NULL;
        for (size_t n = 0; n < num_items; n++, temp = NULL) {
          queue_pop(rule_items, (data_item_t *)&temp);
          LOG_ERROR_IF((temp == NULL), "got null pointer");
          if (temp)
            memcpy(&rule_items_v[n], temp, rule_item_sz);
        }
        LOG_ERROR_IF(!queue_is_empty(rule_items), "rule items left in queue");
        queue_destroy(rule_items, false, NULL);
        rule_items = NULL;

        // enqueue this rule
        filter_rule_t *rule = malloc(sizeof(*rule));
        filter_rule_item_t **items_p = (void *)&rule->items;
        *items_p = rule_items_v;
        size_t *num_items_p = (void *)&rule->num_items;
        *num_items_p = num_items;

        queue_push(rules, (data_item_t){.ptr = rule});
      }

      if (chars_read == -1) {
        LOG_DEBUG("parsed %lu rules", queue_length(rules));
        break;
      }

    } else if (line[0] == '#') {

      continue; // skip comment lines

    } else {

      // process the line to detect a key-value pair and add to current rule

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

        bool mode_include;
        if (strcmp(value, "exclude") == 0) {
          mode_include = false;
        } else if (strcmp(value, "include") == 0) {
          mode_include = true;
        } else {
          LOG_ERROR("invalid mode: %s", value);
          return -1;
        }
        mode_set = true;
        LOG_DEBUG("mode: %s", mode_include ? "include" : "exclude");
        free(key);
        continue;
      }

      // this line defines a key-value pair, so parse out the value and append
      // to rule

      if (rule_items == NULL) {
        // no current rule, create a queue for items for this rule
        LOG_DEBUG("parse rule");
        rule_items = queue_create();
      }

      // add an item to the rule
      filter_rule_item_t *rule_item = malloc(sizeof(*rule_item));
      queue_push(rule_items, (data_item_t){.ptr = rule_item});

      // get a pointer to the const key which we can write through
      filter_key_t *key_p = (void *)&rule_item->key;

      // set the key for this item
      if (strncmp(key, "label", MAXKEY) == 0) {
        *key_p = key_label;
      } else if (strncmp(key, "init", MAXKEY) == 0) {
        *key_p = key_init;
      } else if (strncmp(key, "start", MAXKEY) == 0) {
        *key_p = key_start;
      } else if (strncmp(key, "end", MAXKEY) == 0) {
        *key_p = key_end;
      } else {
        LOG_ERROR(
            "invalid key: %s (must be one of \"label\", \"init\", \"start\", \"end\")",
            key);
        return -1;
      }

      // parse the value based on the key
      switch (rule_item->key) {

      case key_label:

        char *p = calloc(1 + vlen, sizeof(*p));
        char **textp = (void *)&rule_item->value.text;
        memcpy(p, value, vlen);
        *textp = p;

        LOG_DEBUG("  got label: \"%s\"", rule_item->value.text);

        break;

      case key_init:
      case key_start:
      case key_end:

        otter_src_location_t *src = (void *)&rule_item->value.src;

        char sep = ':';
        char *sep_p = NULL;

        if ((sep_p = strrchr(value, sep)) == NULL) {

          // sep not found, value is <file>
          dest = calloc(1 + vlen, sizeof(char));
          memcpy(dest, value, vlen);
          src->file = dest;

        } else {

          char *sub = sep_p + 1;

          // sep is present, value is <file>:<line> or <file>:<func>

          // copy up to sep_p for <file>
          // size_t sz = sep_p - value;
          dest = calloc(1 + sep_p - value, sizeof(char));
          memcpy(dest, value, sep_p - value);
          src->file = dest;

          if ((src->line = (int)strtol(sub, NULL, 10)) != 0) {

            // parsed int, assume <line>
            src->func = NULL;

          } else {

            // assume <func>
            size_t func_name_len = strlen(sep_p + 1); // ...:<func>
            dest = calloc(1 + func_name_len, sizeof(char));
            memcpy(dest, sep_p + 1, func_name_len);
            src->func = dest;
          }
        }

        LOG_DEBUG("  got location:");
        LOG_DEBUG("    file: %s", rule_item->value.src.file);
        LOG_DEBUG("    func: %s", rule_item->value.src.func);
        LOG_DEBUG("    line: %d", rule_item->value.src.line);

        break;

      default:
        LOG_ERROR("unhandled key \"%s\"", key);
      }

      free(key); // TODO: CHECK THIS
    }
  }

  // "rules" is a queue of filter_rule_t*
  size_t num_rules = queue_length(rules);

  // alloc a mutable array of mutable pointers to const rules
  const filter_rule_t **rules_v = malloc(num_rules * sizeof(*rules_v));

  // move the ptr for each rule to the array
  for (int n = 0; n < num_rules; n++) {
    queue_pop(rules, (data_item_t *)&rules_v[n]);
  }
  LOG_ERROR_IF(!queue_is_empty(rules), "rules left in queue");
  queue_destroy(rules, false, NULL);

  // store the number of rules in the array
  size_t *num_rules_parsed = (void *)&(*filt)->num_rules;
  *num_rules_parsed = num_rules;

  // store the array of rules
  // we need *** here to get the address of (*1) the const pointer (*2) to the
  // array of mutable pointers (*3) to const rules
  const filter_rule_t ***rules_p = (void *)&(*filt)->rules;
  *rules_p = rules_v;

  free(line);

  return 0;
}

void trace_filter_fwrite(const trace_filter_t *filter, FILE *stream) {

  if (filter == NULL) {
    return;
  }

  size_t num_rules = filter->num_rules;
  const filter_rule_t *const *rules = filter->rules;
  const filter_rule_t *rule = NULL;

  for (size_t i = 0; i < num_rules; i++) {

    rule = rules[i];
    size_t num_items = rule->num_items;
    fprintf(stream, "rule %lu (%lu items):\n", i + 1, num_items);

    for (size_t j = 0; j < num_items; j++) {

      filter_rule_item_t item = rule->items[j];

      switch (item.key) {

      case key_label:
        fprintf(stream, "  label: \"%s\"\n", item.value.text);
        break;

      case key_init:
        fprintf(stream, "  init: file=%s, func=%s, line=%d\n",
                item.value.src.file, item.value.src.func, item.value.src.line);
        break;

      case key_start:
        fprintf(stream, "  start: file=%s, func=%s, line=%d\n",
                item.value.src.file, item.value.src.func, item.value.src.line);
        break;

      case key_end:
        fprintf(stream, "  end: file=%s, func=%s, line=%d\n",
                item.value.src.file, item.value.src.func, item.value.src.line);
        break;

      default:
        LOG_ERROR("unkown key %d", item.key);
        break;
      }
    }
    fprintf(stream, "\n");
  }

  return;
}
