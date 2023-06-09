#include <string>
#include <map>
#include "public/types/vptr_manager.hpp"

struct vptr_manager {
    using key = std::string;
    using label = void*;
    using map = std::map<key,label>;
    using counter = std::map<key, int>;
    map i_map;
    counter i_count;
};

// C wrappers

vptr_manager* vptr_manager_make() {
    return new vptr_manager();
}

void vptr_manager_delete(vptr_manager* manager, vptr_callback *callback) {
    if (callback) {
        for (auto&[k, count] : manager->i_count) {
            callback(k.c_str(), count);
        }
    }
    delete manager;
}

void vptr_manager_insert_item(vptr_manager* manager, const char* s, void* value) {
    vptr_manager::key key(s);
    manager->i_map[key] = value;
    manager->i_count[key]++;
    return;
}

void vptr_manager_delete_item(vptr_manager* manager, const char* s) {
    manager->i_map.erase(vptr_manager::key(s));
    return;
}

void* vptr_manager_get_item(vptr_manager* manager, const char* s) {
    return manager->i_map[vptr_manager::key(s)];
}

void* vptr_manager_pop_item(vptr_manager* manager, const char* s) {
    vptr_manager::key key(s);
    vptr_manager::label result = manager->i_map[key];
    manager->i_map.erase(key);
    return result;
}
