#include <map>
#include <string>
#include <gtest/gtest.h>
#include "public/types/vptr_manager.hpp"

static int inserted;
static int deleted;
static bool callback_triggered;

vptr_callback mock_vptr_callback;

using string_counter = std::map<std::string, int>;

namespace {
class TestVPtrManager: public testing::Test {
public:
    using key = const char*;
    using value = void*;
protected:
    vptr_manager *m1;

    void SetUp() override {
        m1 = vptr_manager_make();
        inserted = 0;
        deleted = 0;
        callback_triggered = false;
    }

    void SafeDelete(vptr_manager*& manager) {
        vptr_manager_delete(manager);
        manager = nullptr;
    }

    virtual void TearDown() override {
        if (m1) {
            SafeDelete(m1);
        }
        inserted = 0;
        deleted = 0;
        callback_triggered = false;
    }

};

}

/*** TESTS ***/

TEST_F(TestVPtrManager, IsNonNull){
    ASSERT_NE(m1, nullptr);
}

TEST_F(TestVPtrManager, KeyIsInserted){
    int foo;
    void *pExpected = &foo;
    key k = "key<foo>";
    vptr_manager_insert_item(m1, k, pExpected);
    ASSERT_EQ(pExpected, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, KeyIsDeleted){
    int foo;
    void *pFoo = &foo;
    key k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    vptr_manager_delete_item(m1, k);
    ASSERT_EQ(nullptr, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, InsertOverwritesValue){
    int foo, bar;
    void *pFoo = &foo, *pBar = &bar;
    key k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    ASSERT_EQ(pFoo, vptr_manager_get_item(m1, k));
    vptr_manager_insert_item(m1, k, pBar);
    ASSERT_EQ(pBar, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, PoppedKeyIsDeleted) {
    int foo;
    void *pFoo = &foo;
    key k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    ASSERT_EQ(vptr_manager_pop_item(m1, k), pFoo);
    ASSERT_EQ(vptr_manager_pop_item(m1, k), nullptr);
}

TEST_F(TestVPtrManager, CanDeleteAbsentKey) {
    key k = "this key wasn't inserted!";
    vptr_manager_delete_item(m1, k);
    ASSERT_TRUE(true);
}

TEST_F(TestVPtrManager, AbsentKeyIsNull) {
    key k = "some absent key";
    ASSERT_EQ(vptr_manager_get_item(m1, k), nullptr);
}

/*** TEST C FUNCTIONS ***/

TEST_F(TestVPtrManager, IsNonNull_C) {
    m1 = vptr_manager_make();
    ASSERT_NE(nullptr, m1);
}

TEST_F(TestVPtrManager, KeyIsInserted_C) {
    int foo;
    void *pExpected = &foo;
    const char* key = "key<foo>";
    vptr_manager_insert_item(m1, key, pExpected);
    void* pInserted = vptr_manager_get_item(m1, key);
    ASSERT_EQ(pExpected, pInserted);
}

TEST_F(TestVPtrManager, KeyIsDeleted_C) {
    int foo;
    void *pFoo = &foo;
    const char* k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    vptr_manager_delete_item(m1, k);
    ASSERT_EQ(nullptr, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, KeyIsUpdated_C){
    int foo, bar;
    void *pFoo = &foo, *pBar = &bar;
    const char* k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    vptr_manager_insert_item(m1, k, pBar);
    ASSERT_EQ(pBar, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, CanDeleteAbsentKey_C) {
    const char* k = "this key wasn't inserted!";
    vptr_manager_delete_item(m1, k);
    ASSERT_TRUE(true);
}

TEST_F(TestVPtrManager, AbsentKeyIsNull_C) {
    const char* k = "some absent key";
    ASSERT_EQ(nullptr, vptr_manager_get_item(m1, k));
}

TEST_F(TestVPtrManager, CallbackIsTriggered) {
    const char* k = "the key";
    auto callback = [](const char *k, int count, void *triggered) -> void {
        *static_cast<bool*>(triggered) = true;
    };
    vptr_manager_insert_item(m1, k, nullptr);
    vptr_manager_count_inserts(m1, callback, static_cast<void*>(&callback_triggered));
    ASSERT_TRUE(callback_triggered);
}

TEST_F(TestVPtrManager, CallbackCountsInsertions) {
    const char* k = "the key";
    int insertions = 0;
    auto callback = [](const char *k, int count, void *counter) -> void {
        auto total = static_cast<int*>(counter);
        *total = *total + 1;
    };
    vptr_manager_insert_item(m1, k, nullptr);
    vptr_manager_count_inserts(m1, callback, static_cast<void*>(&insertions));
    ASSERT_EQ(insertions, 1);
}

TEST_F(TestVPtrManager, CallbackCountsInsertionsPerKey) {

    // Insert data
    const char* k1 = "the key";
    const char* k2 = "another key";
    string_counter expected, observed;
    expected[k1] = 2;
    expected[k2] = 1;

    for (auto[key, required_count] : expected) {
        while (required_count--) {
            vptr_manager_insert_item(m1, key.c_str(), nullptr);
        }
    }

    auto callback = [](const char *k, int count, void *data) -> void {
        auto& observed = *static_cast<string_counter*>(data);
        observed[k] = count;
    };

    vptr_manager_count_inserts(m1, callback, static_cast<void*>(&observed));
    ASSERT_EQ(observed, expected);
}

TEST_F(TestVPtrManager, CallbackCountsDeletedKeys) {

    // Insert data
    const char* k1 = "the key";
    const char* k2 = "another key";
    string_counter expected, observed;
    expected[k1] = 2;
    expected[k2] = 1;

    for (auto[key, required_count] : expected) {
        while (required_count--) {
            vptr_manager_insert_item(m1, key.c_str(), nullptr);
            vptr_manager_delete_item(m1, key.c_str());
        }
    }

    auto callback = [](const char *k, int count, void *data) -> void {
        auto& observed = *static_cast<string_counter*>(data);
        observed[k] = count;
    };

    vptr_manager_count_inserts(m1, callback, static_cast<void*>(&observed));
    ASSERT_EQ(observed, expected);
}
