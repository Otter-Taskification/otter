#include <iostream>
#include <string>
#include <functional>
#include <gtest/gtest.h>
#include "public/types/string_value_registry.hpp"

template<typename V>
static V registry_label;
static int inserted;
static int deleted;
static bool called_with_nullptr;

template<typename V>
V mock_labeller();

template<typename K, typename V>
void mock_deleter(K key, V value, destructor_data data);

void mock_deleter_cstr(const char* key, uint32_t label, destructor_data data);
void mock_deleter_cstr_increments_int_ptr(const char* key, uint32_t label, destructor_data data);
void mock_deleter_cstr_checks_called_with_nullptr(const char* key, uint32_t label, destructor_data data);

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
        r = registry::make(labeller, destructor, nullptr);
        s = nullptr;
        t = nullptr;
        registry_label<label> = 1;
        inserted = 0;
        deleted = 0;
        called_with_nullptr = false;
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
        called_with_nullptr = false;
    }

};

} // namespace

template<typename V>
V mock_labeller()
{
    inserted ++; return registry_label<V>++;
}

template<typename K, typename V>
void mock_deleter(K key, V value, destructor_data data)
{
    deleted++;
}

void mock_deleter_cstr(const char* key, uint32_t label, destructor_data data)
{
    std::cerr << "mock_deleter_cstr(" << key << ")" << std::endl;
    deleted++;
}

void mock_deleter_cstr_increments_int_ptr(const char* key, uint32_t label, destructor_data data)
{
    auto value = static_cast<int*>(data);
    std::cerr << "mock_deleter_cstr(" << key << ")" << std::endl;
    deleted++;
    (*value)++;
}

void mock_deleter_cstr_checks_called_with_nullptr(const char* key, uint32_t label, destructor_data data)
{
    if (data == nullptr) {
        called_with_nullptr = true;
    }
}


using TestStringRegistryDeath = TestStringRegistry;
using TestStringRegistry_C = TestStringRegistry;

/*** TESTS ***/

TEST_F(TestStringRegistry, IsNonNull){
    ASSERT_NE(r, nullptr);
}

TEST_F(TestStringRegistry, AcceptsNullDeleter){
    s = TestStringRegistry::registry::make(&mock_labeller<TestStringRegistry::label>, nullptr, nullptr);
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
        s = TestStringRegistryDeath::registry::make(nullptr, &mock_deleter<TestStringRegistryDeath::key, TestStringRegistryDeath::label>, nullptr);
    }, ".*");
}

/*** TEST C FUNCTIONS ***/

TEST_F(TestStringRegistry_C, CreateWithNullDeleter){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, nullptr, nullptr);
    ASSERT_NE(t, nullptr);
}

TEST_F(TestStringRegistry_C, CreateWithDeleter){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr, nullptr);
    ASSERT_NE(t, nullptr);
}

TEST_F(TestStringRegistry_C, KeyIsLabelled){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr, nullptr);
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id = t->insert(key);
    ASSERT_EQ(id, 1);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, SameKeySameLabel){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr, nullptr);
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id1 = t->insert(key);
    TestStringRegistry::label id2 = t->insert(key);
    ASSERT_EQ(id1, id2);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, DiffKeyDiffLabel){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr, nullptr);
    TestStringRegistry::key key1 = "foo";
    TestStringRegistry::key key2 = "bar";
    TestStringRegistry::label id1 = t->insert(key1);
    TestStringRegistry::label id2 = t->insert(key2);
    ASSERT_EQ(id1+1, id2);
    TestStringRegistry::SafeDelete(t);
}

TEST_F(TestStringRegistry_C, InsertedEqDeleted){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr, nullptr);
    TestStringRegistry::key keys[] = {"foo", "bar", "baz"};
    for (auto &key : keys)
    {
        t->insert(key);
    }
    ASSERT_EQ(inserted, 3);
    TestStringRegistry::SafeDelete(t);
    ASSERT_EQ(deleted, 3);
}

TEST_F(TestStringRegistry_C, DeleterCalledWithNullData){
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr_checks_called_with_nullptr, nullptr);
    TestStringRegistry::key key = "foo";
    TestStringRegistry::label id = t->insert(key);
    TestStringRegistry::SafeDelete(t);
    ASSERT_TRUE(called_with_nullptr);
}

TEST_F(TestStringRegistry_C, DeleterCalledWithData){
    int count = 0;
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, &mock_deleter_cstr_increments_int_ptr, (destructor_data) &count);
    TestStringRegistry::key keys[] = {"foo", "bar", "baz"};
    for (auto &key : keys)
    {
        t->insert(key);
    }
    TestStringRegistry::SafeDelete(t);
    ASSERT_EQ(count, 3);
}

TEST_F(TestStringRegistry_C, ArgOutlivesScope){
    auto print_values = [](const char* str, uint32_t label, destructor_data data) -> void {
        std::cerr << "(" << label << "," << str << ")" << std::endl;
    };
    t = string_registry_make(&mock_labeller<TestStringRegistry::label>, print_values, nullptr);
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
