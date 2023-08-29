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

typedef union filter_rule_value_t {
  char *text;
  otter_src_location_t src;
} filter_rule_value_t;

typedef struct filter_rule_item_t {
  filter_key_t key;
  int flags;
  filter_rule_value_t value;
} filter_rule_item_t;

int trace_filter_load(trace_filter_t **filter, FILE *filter_file) {

  if ((filter_file == NULL) || (filter == NULL)) {
    return -1;
  }

  LOG_DEBUG("parsing filter file");

  LOG_DEBUG("allocate array of filter rules");

  bool mode_set = false;
  char *line = NULL, *key = NULL, *value = NULL, *dest = NULL;
  int consumed = 0, rules_parsed = 0;
  size_t size = 0;
  otter_queue_t *rule_items = NULL;

  while (true) {

    errno = 0; // want to detect whether size == -1 is error or EOF
    size = getline(&line, &size, filter_file);

    // LOG_INFO("line: [%s]", line);

    if ((size == -1) && (errno != 0)) {
      // error
      LOG_ERROR("failed to read line: %s", strerror(errno));
      break;
    } else if ((size == 1) || (size == -1)) {

      // empty line or EOF, current rule finished
      // TODO: store rule_items somewhere e.g. add it to the filter
      if (rule_items != NULL) {
        LOG_INFO("finished rule with %lu items", queue_length(rule_items));
      }
      rule_items = NULL;

      if (size == -1) {
        LOG_INFO("EOF reached, done parsing file");
        break;
      }

    } else if (line[0] == '#') {
      continue; // skip comment lines
    } else {

      // process the line to detect a key-value pair and add to current rule

      // replace newline
      if (line[size - 1] == '\n') {
        line[size - 1] = '\0';
      }

      LOG_INFO("process line: \"%s\"", line);

      // get key
      int scan_result = 0;
      if ((scan_result = sscanf(line, "%ms %n", &key, &consumed)) == EOF) {
        LOG_ERROR("invalid line: \"%s\"", line);
        return -1;
      }

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
        LOG_DEBUG("include mode: %s", mode_include ? "yes" : "no");
        free(key);
        continue;
      }

      // this line defines a key-value pair, so parse out the value and append
      // to rule

      if (rule_items == NULL) {
        rule_items = queue_create();
      }

      filter_rule_item_t *rule_item = malloc(sizeof(*rule_item));
      queue_push(rule_items, (data_item_t){.ptr = rule_item});

      if (strncmp(key, "label", MAXKEY) == 0) {
        rule_item->key = key_label;
      } else if (strncmp(key, "init", MAXKEY) == 0) {
        rule_item->key = key_init;
      } else if (strncmp(key, "start", MAXKEY) == 0) {
        rule_item->key = key_start;
      } else if (strncmp(key, "end", MAXKEY) == 0) {
        rule_item->key = key_end;
      } else {
        LOG_ERROR(
            "invalid key: %s (must be one of \"label\", \"init\", \"start\", \"end\")",
            key);
        return -1;
      }

      // key is done, parse value

      /*
      key must be one of: "label", "init", "start", "end", "mode"

      if key is "label":

        value is the whole string and is a task label

      if key is "init", "start" or "end":

        if value contains ":":

          value is either <file>:<line> or <file>:<func>:

        else:

          value is <file>

      if key is "mode":

        value must be either "include" or "exclude"

      */

      switch (rule_item->key) {

      case key_label:

        rule_item->value.text = calloc(1 + vlen, sizeof(char));
        memcpy(rule_item->value.text, value, vlen);

        LOG_INFO("got label: \"%s\"", rule_item->value.text);

        break;

      case key_init:
      case key_start:
      case key_end:

        // LOG_INFO("parse location: %s", value);

        char sep = ':';
        char *sep_p = NULL;

        if ((sep_p = strrchr(value, sep)) == NULL) {

          // sep not found, value is <file>
          dest = calloc(1 + vlen, sizeof(char));
          memcpy(dest, value, vlen);
          rule_item->value.src.file = dest;

          // LOG_INFO("got file: %s", rule_item->value.src.file);

        } else {

          char *sub = sep_p + 1;

          // sep is present, value is <file>:<line> or <file>:<func>

          // copy up to sep_p for <file>
          // size_t sz = sep_p - value;
          dest = calloc(1 + sep_p - value, sizeof(char));
          memcpy(dest, value, sep_p - value);
          rule_item->value.src.file = dest;

          if ((rule_item->value.src.line = (int)strtol(sub, NULL, 10)) != 0) {

            // parsed int, assume <line>
            rule_item->value.src.func = NULL;

            // LOG_INFO("got file & line: %s %u", rule_item->value.src.file,
            //          rule_item->value.src.line);

          } else {

            // assume <func>
            size_t func_name_len = strlen(sep_p + 1); // ...:<func>
            dest = calloc(1 + func_name_len, sizeof(char));
            memcpy(dest, sep_p + 1, func_name_len);
            rule_item->value.src.func = dest;

            // LOG_INFO("got file & func: %s %s", rule_item->value.src.file,
            //          rule_item->value.src.func);
          }
        }

        LOG_INFO("location:");
        LOG_INFO("    file: %s", rule_item->value.src.file);
        LOG_INFO("    func: %s", rule_item->value.src.func);
        LOG_INFO("    line: %d", rule_item->value.src.line);

        break;

      default:
        LOG_ERROR("unhandled key \"%s\"", key);
      }

      free(key); // TODO: CHECK THIS
    }
  }

  free(line);

  return 0;
}
