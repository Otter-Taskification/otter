#include <string>
#include <functional>
#include <gtest/gtest.h>
#include <otter/value_registry.hpp>

template<typename V>
static V registry_label;
static int inserted;
static int deleted;

template<typename V>
V mock_labeller();

template<typename K, typename V>
void mock_deleter(K key, V value);

namespace {
class TestStringRegistry: public testing::Test {
public:
    using registry = value_registry<std::string, uint32_t>;
    using key = registry::key;
    using label = registry::label;
protected:
    registry *r;
    registry *s;

    void SetUp() override {
        registry::labelcbk labeller = &mock_labeller<label>;
        registry::destroycbk destructor = &mock_deleter<key, label>;
        r = registry::make(labeller, destructor);
        s = nullptr;
        registry_label<label> = 1;
        inserted = 0;
        deleted = 0;
    }

    void SafeDelete(registry*& reg) {
        delete reg;
        reg = nullptr;
    }

    virtual void TearDown() override {
        SafeDelete(r);
        SafeDelete(s);
        registry_label<label> = 1;
        inserted = 0;
        deleted = 0;
    }

};

} // namespace

template<typename V>
V mock_labeller()
{
    inserted ++; return registry_label<V>++;
}

template<typename K, typename V>
void mock_deleter(K key, V value)
{
    deleted++;
}

using TestStringRegistryDeath = TestStringRegistry;

/*** TESTS ***/

TEST_F(TestStringRegistry, IsNonNull){
    ASSERT_NE(r, nullptr);
}

TEST_F(TestStringRegistry, AcceptsNullDeleter){
    s = TestStringRegistry::registry::make(&mock_labeller<TestStringRegistry::label>, nullptr);
    ASSERT_NE(s, nullptr);
}

TEST_F(TestStringRegistry, KeyIsLabelled){
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id = r->insert(key);
    ASSERT_EQ(id, 1);
}

TEST_F(TestStringRegistry, SameKeySameLabel){
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id1 = r->insert(key);
    TestStringRegistry::label id2 = r->insert(key);
    ASSERT_EQ(id1, id2);
}

TEST_F(TestStringRegistry, DiffKeyDiffLabel){
    TestStringRegistry::key key1 = "foo";
    TestStringRegistry::key key2 = "bar";
    TestStringRegistry::label id1 = r->insert(key1);
    TestStringRegistry::label id2 = r->insert(key2);
    ASSERT_EQ(id1+1, id2);
}

TEST_F(TestStringRegistry, InsertedEqDeleted){
    TestStringRegistry::key keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        r->insert(key);
    }
    ASSERT_EQ(inserted, 3);
    TestStringRegistry::SafeDelete(r);
    ASSERT_EQ(deleted, 3);
}

/*** DEATH TESTS ***/

TEST_F(TestStringRegistryDeath, DeathOnNullLabeller){
    ASSERT_DEATH({
        s = TestStringRegistryDeath::registry::make(
            nullptr,
            &mock_deleter<TestStringRegistryDeath::key, TestStringRegistryDeath::label>
        );
    }, ".*");
}
