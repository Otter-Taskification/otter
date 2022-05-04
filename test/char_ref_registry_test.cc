#include <gtest/gtest.h>
#include <otter/char_ref_registry.hpp>

static uint32_t mock_labeller();
static void mock_deleter(const char*, uint32_t);

static uint32_t label;
static uint32_t inserted;
static uint32_t deleted;

namespace {
class CharRefRegistryTextFxt: public testing::Test {
protected:
    char_ref_registry *r;
    char_ref_registry *s;

    void SetUp() override {
        r = char_ref_registry_make(mock_labeller, mock_deleter);
        s = nullptr;
        label = 1;
        inserted = 0;
        deleted = 0;
    }

    virtual void TearDown() override {
        if (r) {
            char_ref_registry_delete(r);
            r = nullptr;
        }
        if (s) s = nullptr;
        label = 1;
        inserted = 0;
        deleted = 0;
    }

};
}

using CharRefRegistryDeathTextFxt = CharRefRegistryTextFxt;

static uint32_t mock_labeller() { inserted++; return label++; }

static void mock_deleter(const char *key, uint32_t value) { deleted++; }

/*** TESTS ***/

TEST_F(CharRefRegistryTextFxt, IsNonNull){
    ASSERT_NE(r, nullptr);
}

TEST_F(CharRefRegistryTextFxt, AcceptsNullDeleter){
    s = char_ref_registry_make(mock_labeller, nullptr);
    ASSERT_NE(s, nullptr);
    char_ref_registry_delete(s);
}

TEST_F(CharRefRegistryTextFxt, KeyIsLabelled){
    const char *key = "foo";
    uint32_t id = char_ref_registry_insert(r, key);
    ASSERT_EQ(id, 1);
}

TEST_F(CharRefRegistryTextFxt, SameKeySameLabel){
    const char *key = "foo";
    uint32_t id1 = char_ref_registry_insert(r, key);
    uint32_t id2 = char_ref_registry_insert(r, key);
    ASSERT_EQ(id1, id2);
}

TEST_F(CharRefRegistryTextFxt, DiffKeyDiffLabel){
    const char *key1 = "foo";
    const char *key2 = "bar";
    uint32_t id1 = char_ref_registry_insert(r, key1);
    uint32_t id2 = char_ref_registry_insert(r, key2);
    ASSERT_NE(id1, id2);
}

TEST_F(CharRefRegistryTextFxt, InsertedEqDeleted){
    const char *keys[] = {"foo", "bar", "baz"};
    for (auto& key : keys)
    {
        char_ref_registry_insert(r, key);
    }
    ASSERT_EQ(inserted, 3);
    char_ref_registry_delete(r);
    ASSERT_EQ(deleted, 3);
    r = nullptr;
}

/*** DEATH TESTS ***/

TEST_F(CharRefRegistryDeathTextFxt, DeathOnNullLabeller){
    ASSERT_DEATH({
        s = char_ref_registry_make(nullptr, mock_deleter);
    }, ".*");
}
