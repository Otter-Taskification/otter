#include <map>
#include <functional>

// Declare the class template without defining how to instantiate
template<typename KeyType, typename LabelType>
class value_registry {
public:
    using key = KeyType;
    using label = LabelType;
    using map = std::map<key,label>;
    using labelcbk = label(*)(void);
    using destroycbk = void(*)(key, label);
    using destroyfn = std::function<void(key, label)>;

    // ctor
    value_registry(labelcbk getlabel, destroycbk destructor);
    value_registry(labelcbk getlabel, destroyfn destructor);

    // dtor
    ~value_registry();

    // copy
    value_registry(const value_registry& other) = delete;
    value_registry& operator=(const value_registry& other) = delete;

    // move
    value_registry(value_registry&& other) = delete;
    value_registry& operator=(value_registry&& other) = delete;

    static value_registry<key, label>* make(labelcbk getlabel, destroycbk destructor);
    static value_registry<key, label>* make(labelcbk getlabel, destroyfn destructor);
    static void destroy(value_registry<key, label>* r);
    label insert(key k);
private:
    map i_map;
    label i_default_label;
    labelcbk i_get_label;
    destroycbk i_destroy_entry;
    destroyfn i_destroy_entry_fn;
    bool i_have_destroyfn;
};
