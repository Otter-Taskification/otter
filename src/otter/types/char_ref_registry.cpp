#include <cassert>
#include "otter/char_ref_registry.hpp"

class char_ref_registry {
public:
    char_ref_registry(char_ref_registry_label_cbk labeller, char_ref_registry_delete_cbk deleter) :
        delete_key_value_pair(deleter), get_label(labeller)
    {
        assert(labeller != nullptr);
    };
    ~char_ref_registry();
    char_ref_registry(const char_ref_registry&) = delete;
    char_ref_registry(char_ref_registry&&) = delete;
    char_ref_registry& operator=(const char_ref_registry&) = delete;
    char_ref_registry& operator=(char_ref_registry&&) = delete;
    uint32_t insert(const char *);
private:
    char_ref_registry_label_cbk     get_label;
    char_ref_registry_delete_cbk    delete_key_value_pair;
    std::map<const char*, uint32_t> registry_m;
};

char_ref_registry* char_ref_registry_make(
    char_ref_registry_label_cbk  labeller,
    char_ref_registry_delete_cbk deleter)
{
    return new char_ref_registry(labeller, deleter);
}

void char_ref_registry_delete(char_ref_registry *r) {
    delete r;
}

uint32_t char_ref_registry_insert(char_ref_registry *registry, const char *key)
{
    return registry->insert(key);
}

uint32_t char_ref_registry::insert(const char *key) {
    auto label = registry_m[key];
    if (label == 0) { // key not previously added
        label = this->get_label();
        registry_m[key] = label;
    }
    return label;
}

char_ref_registry::~char_ref_registry() {
    if (this->delete_key_value_pair) {
        for (auto&[key, value] : registry_m) {
            this->delete_key_value_pair(key, value);
        }
    }
}
