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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "public/debug.h"
#include "public/otter-common.h"

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
  filter_rule_value_t value;
} filter_rule_item_t;

int trace_filter_load(trace_filter_t **filter, FILE *filter_file) {

  if ((filter_file == NULL) || (filter == NULL)) {
    return -1;
  }

  LOG_DEBUG("parsing filter file");

  bool mode_set = false;
  char *line = NULL, *key = NULL, *value = NULL;
  int consumed = 0;
  size_t size = 0;

  while ((size = getline(&line, &size, filter_file)) != -1) {

    if (line[0] == '#') {
      continue; // skip comment lines
    } else if (size == 1) {

      // current rule finished
      LOG_INFO("%s", "<empty line>");

    } else {

      // replace newline
      if (line[size - 1] == '\n') {
        line[size - 1] = '\0';
      }

      // parse key-value pair and add to current rule
      LOG_INFO("%s", line);

      // get key
      int scan_result = 0;
      if ((scan_result = sscanf(line, "%ms %n", &key, &consumed)) == EOF) {
        LOG_ERROR("invalid line: \"%s\"", line);
        return -1;
      }

      value = &line[consumed];
      LOG_INFO("(result=%d) consumed=%d, key=\"%s\", value=\"%s\"", scan_result,
               consumed, key, value);

      if (strncmp(key, "mode", MAXKEY) == 0) {

        // This line sets the mode, detect mode and continue to next line

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

      filter_rule_item_t rule_item;

      if (strncmp(key, "label", MAXKEY) == 0) {
        rule_item.key = key_label;
      } else if (strncmp(key, "init", MAXKEY) == 0) {
        rule_item.key = key_init;
      } else if (strncmp(key, "start", MAXKEY) == 0) {
        rule_item.key = key_start;
      } else if (strncmp(key, "end", MAXKEY) == 0) {
        rule_item.key = key_end;
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

      switch (rule_item.key) {

      case key_label:

        size_t vlen = strlen(value);
        rule_item.value.text = calloc(1 + vlen, sizeof(char));
        memcpy(rule_item.value.text, value, vlen);
        LOG_INFO("got label: \"%s\"", rule_item.value.text);

        break;

      case key_init:
      case key_start:
      case key_end:
        LOG_INFO("parse location: %s", value);
        break;

      default:
        LOG_ERROR("unhandled key \"%s\"", key);
      }

      free(key);
    }
  }

  free(line);

  return 0;
}
