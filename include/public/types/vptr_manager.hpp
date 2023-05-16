// PUBLIC HEADER
#if defined(__cplusplus)

#include <string>
#include <functional>
#include "public/types/value_registry_template_declarations.hpp"
using vptr_manager = value_registry<std::string, void*>;

#else // C definitions

#include <stdint.h>

// Opaque struct
typedef struct vptr_manager vptr_manager;

#endif

#if defined(__cplusplus)
extern "C" {
#endif

vptr_manager* vptr_manager_make();
void vptr_manager_delete(vptr_manager*);
void vptr_manager_insert_item(vptr_manager*, const char*, void*);
void vptr_manager_delete_item(vptr_manager*, const char*);
void* vptr_manager_get_item(vptr_manager*, const char*);
void* vptr_manager_pop_item(vptr_manager*, const char*);

#if defined(__cplusplus)
}
#endif
