#include <string>
#include <functional>
#include <gtest/gtest.h>
#include <otter/value_registry.hpp>

template<typename V>
static V label;
static int inserted;
static int deleted;

template<typename V>
V mock_labeller();

template<typename K, typename V>
void mock_deleter(K key, V value);

namespace {
class StringRegistryTextFxt: public testing::Test {
public:
    using test_registry_type = value_registry<std::string, uint32_t>;
    using test_key_type = test_registry_type::key_type;
    using test_value_type = test_registry_type::value_type;
protected:
    test_registry_type *r;
    test_registry_type *s;

    void SetUp() override {
        test_registry_type::labeller_type labeller = &mock_labeller<test_value_type>;
        test_registry_type::destructor_type destructor = &mock_deleter<test_key_type, test_value_type>;
        r = test_registry_type::make(labeller, destructor);
        s = nullptr;
        label<test_value_type> = 1;
        inserted = 0;
        deleted = 0;
    }

    virtual void TearDown() override {
        if (r) {
            test_registry_type::destroy(r);
            r = nullptr;
        }
        if (s) s = nullptr;
        label<test_value_type> = 1;
        inserted = 0;
        deleted = 0;
    }

};

} // namespace

template<typename V>
V mock_labeller()
{
    inserted ++; return label<V>++;
}

template<typename K, typename V>
void mock_deleter(K key, V value)
{
    deleted++;
}

using StringRegistryDeathTextFxt = StringRegistryTextFxt;

/*** TESTS ***/

TEST_F(StringRegistryTextFxt, IsNonNull){
    ASSERT_NE(r, nullptr);
}

TEST_F(StringRegistryTextFxt, AcceptsNullDeleter){
    s = StringRegistryTextFxt::test_registry_type::make(&mock_labeller<StringRegistryTextFxt::test_value_type>, nullptr);
    ASSERT_NE(s, nullptr);
    StringRegistryTextFxt::test_registry_type::destroy(s);
}

TEST_F(StringRegistryTextFxt, KeyIsLabelled){
    StringRegistryTextFxt::test_key_type key = "foo";
    StringRegistryTextFxt::test_value_type id = r->insert(key);
    ASSERT_EQ(id, 1);
}

TEST_F(StringRegistryTextFxt, SameKeySameLabel){
    StringRegistryTextFxt::test_key_type key = "foo";
    StringRegistryTextFxt::test_value_type id1 = r->insert(key);
    StringRegistryTextFxt::test_value_type id2 = r->insert(key);
    ASSERT_EQ(id1, id2);
}

TEST_F(StringRegistryTextFxt, DiffKeyDiffLabel){
    StringRegistryTextFxt::test_key_type key1 = "foo";
    StringRegistryTextFxt::test_key_type key2 = "bar";
    StringRegistryTextFxt::test_value_type id1 = r->insert(key1);
    StringRegistryTextFxt::test_value_type id2 = r->insert(key2);
    ASSERT_EQ(id1+1, id2);
}

TEST_F(StringRegistryTextFxt, InsertedEqDeleted){
    StringRegistryTextFxt::test_key_type keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        r->insert(key);
    }
    ASSERT_EQ(inserted, 3);
    StringRegistryTextFxt::test_registry_type::destroy(r);
    ASSERT_EQ(deleted, 3);
    r = nullptr;
}

/*** DEATH TESTS ***/

TEST_F(StringRegistryDeathTextFxt, DeathOnNullLabeller){
    ASSERT_DEATH({
        s = StringRegistryDeathTextFxt::test_registry_type::make(
            nullptr,
            &mock_deleter<StringRegistryDeathTextFxt::test_key_type, StringRegistryDeathTextFxt::test_value_type>
        );
    }, ".*");
}
