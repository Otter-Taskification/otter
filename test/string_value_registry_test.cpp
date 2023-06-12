#include <iostream>
#include <string>
#include <functional>
#include <gtest/gtest.h>
#include "public/types/string_value_registry.hpp"

static uint32_t registry_label;
static int inserted;
static int deleted;
static bool called_with_nullptr;

labeller_fn mock_labeller;
deleter_fn mock_deleter;
deleter_fn mock_deleter_cstr;
deleter_fn mock_deleter_cstr_increments_int_ptr;
deleter_fn mock_deleter_cstr_checks_called_with_nullptr;

namespace {
class TestStringRegistry: public testing::Test {
public:
    using key_type = std::string;
    using label_type = uint32_t;
protected:
    string_registry *r;
    string_registry *s;
    string_registry *t;

    void SetUp() override {
        labeller_fn *labeller = &mock_labeller;
        deleter_fn *deleter = &mock_deleter;
        r = string_registry_make(labeller);
        s = nullptr;
        t = nullptr;
        registry_label = 1;
        inserted = 0;
        deleted = 0;
        called_with_nullptr = false;
    }

    void SafeDelete(string_registry*& reg, deleter_fn *deleter, deleter_data data) {
        if (reg) {
            string_registry_delete(reg, deleter, data);
            reg = nullptr;
        }
    }

    virtual void TearDown() override {
        SafeDelete(r, nullptr, nullptr);
        SafeDelete(s, nullptr, nullptr);
        SafeDelete(t, nullptr, nullptr);
        registry_label = 1;
        inserted = 0;
        deleted = 0;
        called_with_nullptr = false;
    }

};

} // namespace

uint32_t mock_labeller()
{
    inserted ++; return registry_label++;
}

void mock_deleter(const char *key, uint32_t value, deleter_data data)
{
    deleted++;
}

void mock_deleter_cstr(const char* key, uint32_t label, deleter_data data)
{
    std::cerr << "mock_deleter_cstr(" << key << ")" << std::endl;
    deleted++;
}

void mock_deleter_cstr_increments_int_ptr(const char* key, uint32_t label, deleter_data data)
{
    auto value = static_cast<int*>(data);
    std::cerr << "mock_deleter_cstr(" << key << ")" << std::endl;
    deleted++;
    (*value)++;
}

void mock_deleter_cstr_checks_called_with_nullptr(const char* key, uint32_t label, deleter_data data)
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
    s = string_registry_make(mock_labeller);
    ASSERT_NE(s, nullptr);
}

TEST_F(TestStringRegistry, KeyIsLabelled){
    const char *key = "foo";
    TestStringRegistry::label_type label = string_registry_insert(r, key);
    ASSERT_EQ(label, 1);
}

TEST_F(TestStringRegistry, SameKeySameLabel){
    const char *key = "foo";
    TestStringRegistry::label_type label1 = string_registry_insert(r, key);
    TestStringRegistry::label_type label2 = string_registry_insert(r, key);
    ASSERT_EQ(label1, label2);
}

TEST_F(TestStringRegistry, DiffKeyDiffLabel){
    const char *key1 = "foo";
    const char *key2 = "bar";
    TestStringRegistry::label_type label1 = string_registry_insert(r, key1);
    TestStringRegistry::label_type label2 = string_registry_insert(r, key2);
    ASSERT_NE(label1, label2);
}

TEST_F(TestStringRegistry, InsertedEqDeleted){
    const char *keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        string_registry_insert(r, key);
    }
    ASSERT_EQ(inserted, 3);
    TestStringRegistry::SafeDelete(r, mock_deleter, nullptr);
    ASSERT_EQ(deleted, 3);
}
TEST_F(TestStringRegistryDeath, DeathOnNullLabeller){
    ASSERT_DEATH({
        s = string_registry_make(nullptr);
    }, ".*");
}

TEST_F(TestStringRegistry_C, KeyIsLabelled){
    t = string_registry_make(mock_labeller);
    const char * key = "foo";
    TestStringRegistry::label_type id = string_registry_insert(t,key);
    ASSERT_EQ(id, 1);
    TestStringRegistry::SafeDelete(t, nullptr, nullptr);
}

TEST_F(TestStringRegistry_C, SameKeySameLabel){
    t = string_registry_make(mock_labeller);
    const char * key = "foo";
    TestStringRegistry::label_type id1 = string_registry_insert(t,key);
    TestStringRegistry::label_type id2 = string_registry_insert(t,key);
    ASSERT_EQ(id1, id2);
    TestStringRegistry::SafeDelete(t, nullptr, nullptr);
}

TEST_F(TestStringRegistry_C, DiffKeyDiffLabel){
    t = string_registry_make(mock_labeller);
    const char * key1 = "foo";
    const char * key2 = "bar";
    TestStringRegistry::label_type id1 = string_registry_insert(t,key1);
    TestStringRegistry::label_type id2 = string_registry_insert(t,key2);
    ASSERT_EQ(id1+1, id2);
    TestStringRegistry::SafeDelete(t, nullptr, nullptr);
}

TEST_F(TestStringRegistry_C, InsertedEqDeleted){
    t = string_registry_make(mock_labeller);
    const char * keys[] = {"foo", "bar", "baz"};
    for (auto &key : keys)
    {
        string_registry_insert(t,key);
    }
    ASSERT_EQ(inserted, 3);
    TestStringRegistry::SafeDelete(t, mock_deleter, nullptr);
    ASSERT_EQ(deleted, 3);
}

TEST_F(TestStringRegistry_C, DeleterCalledWithNullData){
    t = string_registry_make(mock_labeller);
    const char * key = "foo";
    TestStringRegistry::label_type id = string_registry_insert(t,key);
    TestStringRegistry::SafeDelete(t, mock_deleter_cstr_checks_called_with_nullptr, nullptr);
    ASSERT_TRUE(called_with_nullptr);
}

TEST_F(TestStringRegistry_C, DeleterCalledWithData){
    int count = 0;
    t = string_registry_make(mock_labeller);
    const char * keys[] = {"foo", "bar", "baz"};
    for (auto &key : keys)
    {
        string_registry_insert(t,key);
    }
    TestStringRegistry::SafeDelete(t, mock_deleter_cstr_increments_int_ptr, (deleter_data) &count);
    ASSERT_EQ(count, 3);
}

TEST_F(TestStringRegistry_C, ArgOutlivesScope){
    auto print_values = [](const char* str, uint32_t label, deleter_data data) -> void {
        std::cerr << "(" << label << "," << str << ")" << std::endl;
    };
    t = string_registry_make(mock_labeller);
    {
        const char *s1 = "foo";
        const char *s2 = "bar";
        const char *s3 = "baz";
        string_registry_insert(t, s1);
        string_registry_insert(t, s2);
        string_registry_insert(t, s3);
    }
    TestStringRegistry::SafeDelete(t, print_values, nullptr);
    ASSERT_EQ(inserted, 3);
    ASSERT_EQ(deleted, 0);
}
