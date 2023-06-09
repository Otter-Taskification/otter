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

/**
 * @brief Create a `vptr_manager` which maps keys (`const char*`) to values
 * (`void*`).
 * 
 * @return vptr_manager* 
 */
vptr_manager* vptr_manager_make();

/**
 * @brief Delete a `vptr_manager`, applying an optional callback to each
 * key-value pair stored.
 * 
 */
void vptr_manager_delete(vptr_manager*, void(*)(const char*, int));

/**
 * @brief Insert a key-value pair.
 * 
 */
void vptr_manager_insert_item(vptr_manager*, const char*, void*);

/**
 * @brief Delete the value mapped by the given key.
 * 
 */
void vptr_manager_delete_item(vptr_manager*, const char*);

/**
 * @brief Get the value mapped by the given key.
 * 
 * @return void*
 */
void* vptr_manager_get_item(vptr_manager*, const char*);

/**
 * @brief Get the value mapped by the given key and erase it from the map.
 * 
 * @return void* 
 */
void* vptr_manager_pop_item(vptr_manager*, const char*);

#if defined(__cplusplus)
}
#endif
