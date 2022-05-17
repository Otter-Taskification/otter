#include <ostream>
#include <tuple>
#include <map>
#include <string>
#include <cassert>

template<typename K, typename V> // e.g. key=std::string, value=uint32_t
class value_registry {
public:
    using map_type = std::map<K,V>;
    using key_type = K;
    using value_type = V;
    using labeller_type = V(*)(void);
    using destructor_type = void(*)(K, V);
    value_registry(labeller_type labeller, destructor_type destructor) :
        i_get_label{labeller},
        i_destroy_entry(destructor),
        i_default_label{},
        i_map{}
    {
        assert(labeller != nullptr);
    };
    ~value_registry() {
        if (this->i_destroy_entry) {
            for (auto&[key, value] : i_map) {
                this->i_destroy_entry(key, value);
            }
        }
    }
    static value_registry<K, V>* make(labeller_type labeller, destructor_type destructor) { return new value_registry<K, V>(labeller, destructor); }
    static void destroy(value_registry<K, V>* r) { delete r; }
    value_type insert(key_type key) {
        auto label = i_map[key];
        if (label == i_default_label) {
            label = this->i_get_label();
            i_map[key] = label;
        }
        return label;
    }
private:
    map_type i_map;
    value_type i_default_label;
    labeller_type i_get_label;
    destructor_type i_destroy_entry;
};
