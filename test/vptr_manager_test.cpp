#include <gtest/gtest.h>
#include "public/types/vptr_manager.hpp"

static int inserted;
static int deleted;

namespace {
class TestVPtrManager: public testing::Test {
public:
    using key = vptr_manager::key;
    using value = vptr_manager::label;
protected:
    vptr_manager *m1;

    void SetUp() override {
        m1 = new vptr_manager(nullptr, nullptr, nullptr);
        inserted = 0;
        deleted = 0;
    }

    void SafeDelete(vptr_manager*& manager) {
        delete manager;
        manager = nullptr;
    }

    virtual void TearDown() override {
        SafeDelete(m1);
        inserted = 0;
        deleted = 0;
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
    m1->insert_key_value_pair(k, pExpected);
    void *pInserted = m1->get_value(k);
    ASSERT_EQ(pExpected, pInserted);
}

TEST_F(TestVPtrManager, KeyIsDeleted){
    int foo;
    void *pFoo = &foo;
    key k = "key<foo>";
    m1->insert_key_value_pair(k, pFoo);
    m1->remove_key(k);
    void *pInserted = m1->get_value(k);
    ASSERT_EQ(nullptr, pInserted);
}

TEST_F(TestVPtrManager, KeyIsUpdated){
    int foo, bar;
    void *pFoo = &foo, *pBar = &bar;
    key k = "key<foo>";
    m1->insert_key_value_pair(k, pFoo);
    m1->insert_key_value_pair(k, pBar);
    void *pInserted = m1->get_value(k);
    ASSERT_EQ(pBar, pInserted);
}

TEST_F(TestVPtrManager, CanDeleteAbsentKey) {
    key k = "this key wasn't inserted!";
    m1->remove_key(k);
    ASSERT_TRUE(true);
}

TEST_F(TestVPtrManager, AbsentKeyIsNull) {
    key k = "some absent key";
    void *pResult = m1->get_value(k);
    ASSERT_EQ(pResult, nullptr);
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
    void *pInserted = vptr_manager_get_item(m1, k);
    ASSERT_EQ(nullptr, pInserted);
}

TEST_F(TestVPtrManager, KeyIsUpdated_C){
    int foo, bar;
    void *pFoo = &foo, *pBar = &bar;
    const char* k = "key<foo>";
    vptr_manager_insert_item(m1, k, pFoo);
    vptr_manager_insert_item(m1, k, pBar);
    void *pInserted = vptr_manager_get_item(m1, k);
    ASSERT_EQ(pBar, pInserted);
}

TEST_F(TestVPtrManager, CanDeleteAbsentKey_C) {
    const char* k = "this key wasn't inserted!";
    vptr_manager_delete_item(m1, k);
    ASSERT_TRUE(true);
}

TEST_F(TestVPtrManager, AbsentKeyIsNull_C) {
    const char* k = "some absent key";
    void *pResult = vptr_manager_get_item(m1, k);
    ASSERT_EQ(pResult, nullptr);
}
