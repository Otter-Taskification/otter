#include <stdint.h>

typedef struct string_registry string_registry;

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t(labeller_fn)(void);
typedef void* deleter_data;
typedef void(deleter_fn)(const char*, uint32_t, deleter_data);

string_registry* string_registry_make(labeller_fn*);
void string_registry_delete(string_registry*, deleter_fn*, deleter_data);
uint32_t string_registry_insert(string_registry*, const char*);

#if defined(__cplusplus)
}
#endif
