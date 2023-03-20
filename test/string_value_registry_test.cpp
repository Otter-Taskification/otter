#include <iostream>
#include <string>
#include <functional>
#include <gtest/gtest.h>
#include "public/types/string_value_registry.hpp"

template<typename V>
static V registry_label;
static int inserted;
static int deleted;

template<typename V>
V mock_labeller();

template<typename K, typename V>
void mock_deleter(K key, V value);

void mock_deleter_cstr(const char* key, uint32_t label);

namespace {
class TestStringRegistry: public testing::Test {
public:
    using registry = string_registry;
    using key = registry::key;
    using label = registry::label;
protected:
    registry *r;
    registry *s;
    registry *t;

    void SetUp() override {
        registry::labelcbk labeller = &mock_labeller<label>;
        registry::destroycbk destructor = &mock_deleter<key, label>;
        r = registry::make(labeller, destructor);
        s = nullptr;
        t = nullptr;
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
        SafeDelete(t);
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

void mock_deleter_cstr(const char* key, uint32_t label)
{
    std::cerr << "mock_deleter_cstr(" << key << ")" << std::endl;
    deleted++;
}

using TestStringRegistryDeath = TestStringRegistry;
using TestStringRegistry_C = TestStringRegistry;

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

/*** TEST C FUNCTIONS ***/

TEST_F(TestStringRegistry_C, CreateWithNullDeleter){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, nullptr);
    ASSERT_NE(t, nullptr);
}

TEST_F(TestStringRegistry_C, CreateWithDeleter){
    t = string_registry_make(
        &mock_labeller<TestStringRegistry::label>,
        &mock_deleter_cstr
    );
    ASSERT_NE(t, nullptr);
}

TEST_F(TestStringRegistry_C, KeyIsLabelled){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>,&mock_deleter_cstr);
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id = t->insert(key);
    ASSERT_EQ(id, 1);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, SameKeySameLabel){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>,&mock_deleter_cstr);
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id1 = t->insert(key);
    TestStringRegistry::label id2 = t->insert(key);
    ASSERT_EQ(id1, id2);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, DiffKeyDiffLabel){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>,&mock_deleter_cstr);
    TestStringRegistry::key key1 = "foo";
    TestStringRegistry::key key2 = "bar";
    TestStringRegistry::label id1 = t->insert(key1);
    TestStringRegistry::label id2 = t->insert(key2);
    ASSERT_EQ(id1+1, id2);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, InsertedEqDeleted){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>,&mock_deleter_cstr);
    TestStringRegistry::key keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        t->insert(key);
    }
    ASSERT_EQ(inserted, 3);
    TestStringRegistry::SafeDelete(t);
    ASSERT_EQ(deleted, 3);
}

TEST_F(TestStringRegistry_C, ArgOutlivesScope){
    auto print_values = [](const char* str, uint32_t label) -> void {
        std::cerr << "(" << label << "," << str << ")" << std::endl;
    };
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, print_values);
    {
        std::string s1 = "foo";
        std::string s2 = "bar";
        std::string s3 = "baz";
        string_registry_insert(t, s1.c_str());
        string_registry_insert(t, s2.c_str());
        string_registry_insert(t, s3.c_str());
    }
    TestStringRegistry::SafeDelete(t);
    ASSERT_EQ(inserted, 3);
    ASSERT_EQ(deleted, 0);
}
