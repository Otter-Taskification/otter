#include <functional>
#include <map>

// Declare the class template without defining how to instantiate
template <typename KeyType, typename LabelType> class value_registry {
public:
  using key = KeyType;
  using label = LabelType;
  using map = std::map<key, label>;
  using counter = std::map<key, int>;
  using labelcbk = label (*)(void);
  using destructor_data = void *;
  using destroycbk = void (*)(key, label, int, destructor_data);
  using destroyfn = std::function<void(key, label, int, destructor_data)>;
  using countcbk = void (*)(const char *, int);

  // ctor
  value_registry(labelcbk getlabel, destroycbk destructor,
                 destructor_data data);
  value_registry(labelcbk getlabel, destroyfn destructor, destructor_data data);

  // dtor
  ~value_registry();

  // copy
  value_registry(const value_registry &other) = delete;
  value_registry &operator=(const value_registry &other) = delete;

  // move
  value_registry(value_registry &&other) = delete;
  value_registry &operator=(value_registry &&other) = delete;

  static value_registry<key, label> *
  make(labelcbk getlabel, destroycbk destructor, destructor_data data);
  static value_registry<key, label> *
  make(labelcbk getlabel, destroyfn destructor, destructor_data data);
  static void destroy(value_registry<key, label> *r);
  label insert(key k);
  void insert_key_value_pair(key k, label l);
  label get_value(key k);
  label pop_value(key k);
  void remove_key(key k);
  void apply_callback(countcbk cbk);

private:
  map i_map;
  counter i_count;
  label i_default_label;
  labelcbk i_get_label;
  destroycbk i_destroy_entry;
  destroyfn i_destroy_entry_fn;
  bool i_have_destroyfn;
  destructor_data i_destructor_data;
};
