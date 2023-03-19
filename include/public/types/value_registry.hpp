#include <ostream>
#include <tuple>
#include <map>
#include <string>
#include <cassert>

template<typename KeyType, typename LabelType> // e.g. key=std::string, value=uint32_t
class value_registry {
public:
    using key = KeyType;
    using label = LabelType;
    using map = std::map<key,label>;
    using labelcbk = label(*)(void);
    using destroycbk = void(*)(key, label);
    value_registry(labelcbk getlabel, destroycbk destructor) :
        i_get_label{getlabel},
        i_destroy_entry(destructor),
        i_default_label{},
        i_map{}
    {
        assert(i_get_label != nullptr);
    };
    ~value_registry() {
        if (this->i_destroy_entry) {
            for (auto&[key, value] : i_map) {
                this->i_destroy_entry(key, value);
            }
        }
    }
    static value_registry<key, label>* make(labelcbk getlabel, destroycbk destructor) { return new value_registry<key, label>(getlabel, destructor); }
    static void destroy(value_registry<key, label>* r) { delete r; }
    label insert(key k) {
        auto label = i_map[k];
        if (label == i_default_label) {
            label = this->i_get_label();
            i_map[k] = label;
        }
        return label;
    }
private:
    map i_map;
    label i_default_label;
    labelcbk i_get_label;
    destroycbk i_destroy_entry;
};
