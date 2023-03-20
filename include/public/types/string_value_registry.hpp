// PUBLIC HEADER
#if defined(__cplusplus)

#include <string>
#include <functional>
#include "public/types/value_registry_template_declarations.hpp"
using string_registry = value_registry<std::string, uint32_t>;
// using string_destroy_fn = std::function<void(const char *, uint32_t)>;

#else // C definitions

#include <stdint.h>

// Opaque struct
typedef struct string_registry string_registry;

#endif

#if defined(__cplusplus)
extern "C" {
#endif

// Callback types
typedef uint32_t(*labelcbk)(void);
typedef void(*destroycbk)(const char*, uint32_t);

// Defined elsewhere
string_registry* string_registry_make(labelcbk, destroycbk);
void string_registry_delete(string_registry*);
uint32_t string_registry_insert(string_registry*, const char*);

#if defined(__cplusplus)
}
#endif
