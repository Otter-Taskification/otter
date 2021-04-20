#if !defined(OMPT_CORE_H)
#define OMPT_CORE_H

/* Apply a macro to each of the possible values returned when setting a callback
   through ompt_set_callback */
#define FOREACH_OMPT_SET_RESULT(macro, result, event)                   \
    macro(ompt_set_error,             0, result, event)                 \
    macro(ompt_set_never,             1, result, event)                 \
    macro(ompt_set_impossible,        2, result, event)                 \
    macro(ompt_set_sometimes,         3, result, event)                 \
    macro(ompt_set_sometimes_paired,  4, result, event)                 \
    macro(ompt_set_always,            5, result, event)

/* Print the result of attempting to set a callback. Useful as not all callbacks
   may be implemented */
#define print_matching_set_result(name, value, result, event)                  \
    do {                                                                       \
        if ((result == name))                                                  \
            fprintf(stderr, "%-32s -> %s (%d)\n", #event, #name, result);      \
    } while(0);

/* Submit implemented callbacks to OMP and report the result */
#define set_callback(event, callback, id)                                      \
    do {                                                                       \
        if(callbacks.on_##event) {                                             \
            ompt_set_result_t r = ompt_set_callback(                           \
                event, (ompt_callback_t) callbacks.on_##event);                \
            FOREACH_OMPT_SET_RESULT(print_matching_set_result, r, event);      \
        }                                                                      \
    } while(0);

#endif // OMPT_CORE_H
