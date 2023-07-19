#include <stdint.h>

typedef struct string_registry string_registry;

#if defined(__cplusplus)
extern "C" {
#endif

typedef uint32_t(labeller_fn)(void);
typedef void(string_registry_callback)(const char *, uint32_t, void *);

string_registry *string_registry_make(labeller_fn *);
void string_registry_apply(string_registry *, string_registry_callback *,
                           void *);
void string_registry_delete(string_registry *);
uint32_t string_registry_insert(string_registry *, const char *);

#if defined(__cplusplus)
}
#endif
