#ifdef __cplusplus
#include <ostream>
#include <cstdint>
#include <tuple>
#include <map>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus

    // Opaque class which maps const char* to uint32_t
    class char_ref_registry;

    // A callback used to get a new uint32_t label when inserting a new key
    using char_ref_registry_label_cbk = uint32_t (*)(void);

    // A callback to be applied to each key-value pair in the registry when it is
    // deleted
    using char_ref_registry_delete_cbk = void (*)(const char*, uint32_t);

#else

    typedef struct char_ref_registry char_ref_registry;

    // A callback used to get a new value when inserting a new key
    typedef uint32_t (*char_ref_registry_label_cbk)(void);

    // A callback to be applied to each key-value pair in the registry when it is
    // deleted
    typedef void (*char_ref_registry_delete_cbk)(const char*, uint32_t);

#endif // #ifdef __cplusplus

#ifdef __cplusplus
extern "C" {
#endif

// Make a new char_ref_registry, injecting behaviour with callbacks
char_ref_registry *char_ref_registry_make(char_ref_registry_label_cbk, char_ref_registry_delete_cbk);

// Delete a char_ref_registry
void char_ref_registry_delete(char_ref_registry *);

// Insert a key, returning the new value, or the existing value if already in the registry
uint32_t char_ref_registry_insert(char_ref_registry *, const char *);

#ifdef __cplusplus
}
#endif
