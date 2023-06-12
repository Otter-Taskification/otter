#include <string>
#include <map>
#include "public/types/vptr_manager.hpp"

struct vptr_manager {
    using mapping = std::map<std::string, void*>;
    mapping i_map;
    std::map<mapping::key_type, int> i_count;
};

// C wrappers

vptr_manager* vptr_manager_make() {
    return new vptr_manager();
}

void vptr_manager_count_inserts(vptr_manager* manager, vptr_callback *callback, void* data) {
    if (callback) {
        for (auto&[k, count] : manager->i_count) {
            callback(k.c_str(), count, data);
        }
    }
}

void vptr_manager_delete(vptr_manager* manager) {
    delete manager;
}

void vptr_manager_insert_item(vptr_manager* manager, const char* s, void* value) {
    vptr_manager::mapping::key_type key(s);
    manager->i_map[key] = value;
    manager->i_count[key]++;
    return;
}

void vptr_manager_delete_item(vptr_manager* manager, const char* s) {
    manager->i_map.erase(vptr_manager::mapping::key_type(s));
    return;
}

void* vptr_manager_get_item(vptr_manager* manager, const char* s) {
    return manager->i_map[vptr_manager::mapping::key_type(s)];
}

void* vptr_manager_pop_item(vptr_manager* manager, const char* s) {
    vptr_manager::mapping::key_type key(s);
    vptr_manager::mapping::mapped_type value = manager->i_map[key];
    manager->i_map.erase(key);
    return value;
}
