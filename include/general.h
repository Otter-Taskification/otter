#if !defined(OTTER_MACROS_GENERAL_H)
#define OTTER_MACROS_GENERAL_H

#define STR_EQUAL(a,b) (!strcmp(a,b))

#define min(a,b) ((a)<(b)?(a):(b))

#define TO_STRING_INT(...) #__VA_ARGS__

#define TO_STRING(...) TO_STRING_INT(__VA_ARGS__)

#endif // OTTER_MACROS_GENERAL_H
