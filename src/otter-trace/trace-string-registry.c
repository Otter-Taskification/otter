/**
 * @file trace-string-registry.c
 * @author Adam Tuft (adam.s.tuft@durham.ac.uk)
 * @brief Instantiate & manage a string registry for an OTF2 archive
 * @version 0.2.0
 * @date 2022-10-12
 * 
 * @copyright Copyright (c) 2022, Adam Tuft. All rights reserved.
 * 
 */

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "otter-trace/trace-string-registry.h"

/* Reference to global string registry */
static string_registry *Registry = NULL;

/* Mutex for thread-safe access to global string registry */
static pthread_mutex_t lock_string_registry = PTHREAD_MUTEX_INITIALIZER;

void trace_init_str_registry(string_registry *registry)
{
    assert(registry != NULL);
    Registry = registry;
}

void trace_destroy_str_registry(void)
{
    string_registry_delete(Registry);
}

string_registry *trace_get_str_registry(void)
{
    return Registry;
}

uint32_t trace_register_string(const char *str)
{
    return string_registry_insert(Registry, str);
}

uint32_t trace_register_string_with_lock(const char *str)
{
    pthread_mutex_lock(&lock_string_registry);
    uint32_t label = trace_register_string(str);
    pthread_mutex_unlock(&lock_string_registry);
    return label;
}
