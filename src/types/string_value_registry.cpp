#include <iostream>
#include <cassert>
#include <string>
#include <functional>
#include "public/types/string_value_registry.hpp"

// Defines template & explicitly instantiates the specialisations required by public header
template<>
string_registry::value_registry(string_registry::labelcbk getlabel, string_registry::destroycbk destructor, destructor_data data) :
    i_map{},
    i_count{},
    i_default_label{},
    i_get_label{getlabel},
    i_destroy_entry{},
    i_destroy_entry_fn{},
    i_have_destroyfn{},
    i_destructor_data{data}
{
    assert(i_get_label != nullptr);
    if (destructor != nullptr)
    {
        // promote to std::function and set i_have_destroyfn
        i_destroy_entry_fn = [destructor](string_registry::key key, string_registry::label label, int count, destructor_data data) -> void { destructor(key.c_str(), label, count, data); };
        i_have_destroyfn = true;
    }
};

template<>
string_registry::value_registry(string_registry::labelcbk getlabel, string_registry::destroyfn destructor, destructor_data data) :
    i_map{},
    i_count{},
    i_default_label{},
    i_get_label{getlabel},
    i_destroy_entry{},
    i_destroy_entry_fn{destructor},
    i_have_destroyfn{true},
    i_destructor_data{data}
{

}

template<>
string_registry::~value_registry()
{
    if (this->i_have_destroyfn) {
        for (auto&[key, value] : i_map) {
            this->i_destroy_entry_fn(key, value, -1, i_destructor_data);
        }
    }
}

template<>
string_registry* string_registry::make(string_registry::labelcbk getlabel, string_registry::destroycbk destructor, destructor_data data)
{
    return new string_registry(getlabel, destructor, data);
}

template<>
string_registry* string_registry::make(string_registry::labelcbk getlabel, string_registry::destroyfn destructor, destructor_data data)
{
    return new string_registry(getlabel, destructor, data);
}

template<>
void string_registry::destroy(string_registry* self)
{
    delete self;
}

template<>
string_registry::label string_registry::insert(string_registry::key str)
{
    auto label = i_map[str];
    if (label == i_default_label) {
        label = this->i_get_label();
        i_map[str] = label;
    }
    return label;
}

// C wrappers

string_registry* string_registry_make(labelcbk getlabel, destroycbk destructor, destructor_data data)
{
    if (destructor) {
        auto tmp = [destructor](string_registry::key key, string_registry::label label, int count, destructor_data data) -> void { destructor(key.c_str(), label, count, data); };
        string_registry::destroyfn destructor_f{tmp};
        return string_registry::make(getlabel, destructor_f, data);
    } else {
        return string_registry::make(getlabel, nullptr, data);
    }
}

void string_registry_delete(string_registry* registry)
{
    string_registry::destroy(registry);
}

string_registry::label string_registry_insert(string_registry* registry, const char* str)
{
    return registry->insert(std::string{str});
}
