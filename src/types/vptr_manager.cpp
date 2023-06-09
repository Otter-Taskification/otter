#include <string>
#include "public/types/vptr_manager.hpp"

using key_type = vptr_manager::key;
using value_type = vptr_manager::label;

template<>
vptr_manager::value_registry(labelcbk getlabel, destroycbk destructor, destructor_data data) {

}

template<>
vptr_manager::~value_registry() {

}

template<>
void vptr_manager::insert_key_value_pair(key_type key, value_type value) {
    i_map[key] = value;
    i_count[key]++;
    return;
}

template<>
value_type vptr_manager::get_value(key_type key) {
    return i_map[key];
}

template<>
void vptr_manager::remove_key(key_type key) {
    i_map.erase(key);
    return;
}

template<>
value_type vptr_manager::pop_value(key_type key) {
    value_type value = i_map[key];
    this->remove_key(key);
    return value;
}

template<>
void vptr_manager::apply_callback(countcbk callback) {
    if (callback) {
        for (auto&[key, count] : i_count) {
            callback(key.c_str(), count);
        }
    }
    return;
}

// C wrappers

vptr_manager* vptr_manager_make() {
    return new vptr_manager(nullptr, nullptr, nullptr);
}

void vptr_manager_delete(vptr_manager* manager, void(*callback)(const char*, int)) {
    if (callback) {
        manager->apply_callback(callback);
    }
    delete manager;
}

void vptr_manager_insert_item(vptr_manager* manager, const char* key, void* value) {
    manager->insert_key_value_pair(std::string(key), value);
    return;
}

void vptr_manager_delete_item(vptr_manager* manager, const char* key) {
    manager->remove_key(key);
    return;
}

void* vptr_manager_get_item(vptr_manager* manager, const char* key) {
    return manager->get_value(key);
}

void* vptr_manager_pop_item(vptr_manager* manager, const char* key) {
    return manager->pop_value(key);
}
