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
class TestValueRegistry: public testing::Test {
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

using TestValueRegistryDeath = TestValueRegistry;

/*** TESTS ***/

TEST_F(TestValueRegistry, IsNonNull){
    ASSERT_NE(r, nullptr);
}

TEST_F(TestValueRegistry, AcceptsNullDeleter){
    s = TestValueRegistry::registry::make(&mock_labeller<TestValueRegistry::label>, nullptr);
    ASSERT_NE(s, nullptr);
}

TEST_F(TestValueRegistry, KeyIsLabelled){
    TestValueRegistry::key key = "foo";
    TestValueRegistry::label id = r->insert(key);
    ASSERT_EQ(id, 1);
}

TEST_F(TestValueRegistry, SameKeySameLabel){
    TestValueRegistry::key key = "foo";
    TestValueRegistry::label id1 = r->insert(key);
    TestValueRegistry::label id2 = r->insert(key);
    ASSERT_EQ(id1, id2);
}

TEST_F(TestValueRegistry, DiffKeyDiffLabel){
    TestValueRegistry::key key1 = "foo";
    TestValueRegistry::key key2 = "bar";
    TestValueRegistry::label id1 = r->insert(key1);
    TestValueRegistry::label id2 = r->insert(key2);
    ASSERT_EQ(id1+1, id2);
}

TEST_F(TestValueRegistry, InsertedEqDeleted){
    TestValueRegistry::key keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        r->insert(key);
    }
    ASSERT_EQ(inserted, 3);
    TestValueRegistry::SafeDelete(r);
    ASSERT_EQ(deleted, 3);
}

/*** DEATH TESTS ***/

TEST_F(TestValueRegistryDeath, DeathOnNullLabeller){
    ASSERT_DEATH({
        s = TestValueRegistryDeath::registry::make(
            nullptr,
            &mock_deleter<TestValueRegistryDeath::key, TestValueRegistryDeath::label>
        );
    }, ".*");
}
