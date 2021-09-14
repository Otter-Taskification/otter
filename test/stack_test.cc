#include <gtest/gtest.h>
#include <cstddef>
#include "otter/stack.h"

void mock_data_destructor(void *ptr);

static int count_destructor_calls;

namespace {
class StackTestFxt : public testing::Test {
    protected:

    otter_stack_t *s1;
    otter_stack_t *s2;
    otter_stack_t *s3;

    void SetUp() override {
        s1 = stack_create();
        s2 = stack_create();
        s3 = stack_create();
        count_destructor_calls = 0;
    }

    virtual void TearDown() override {
        stack_destroy(s1, false, nullptr);
        stack_destroy(s2, false, nullptr);
        stack_destroy(s3, false, nullptr);
        count_destructor_calls = 0;
    }

};
}

// Helper functions
void mock_data_destructor(void *ptr) {
    count_destructor_calls++;
}

TEST_F(StackTestFxt, IsNonNull) {
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    ASSERT_NE(s3, nullptr);
}

// Push

TEST_F(StackTestFxt, PushNullStackIsFalse) {
    data_item_t item {.value = 1};
    ASSERT_FALSE(stack_push(nullptr, item));
}

TEST_F(StackTestFxt, PushNonNullStackIsTrue) {
    data_item_t item {.value = 1};
    ASSERT_TRUE(stack_push(s1, item));
}

// Pop

TEST_F(StackTestFxt, PopNullStackIsFalse) {
    data_item_t item2;
    ASSERT_FALSE(stack_pop(nullptr, &item2));
}

TEST_F(StackTestFxt, PopNonNullEmptyStackIsFalse) {
    data_item_t item2;
    ASSERT_FALSE(stack_pop(s1, &item2));
}

TEST_F(StackTestFxt, PopNonNullNonEmptyStackIsTrue) {
    data_item_t item1 {.value = 1};
    ASSERT_TRUE(stack_push(s1, item1));
    data_item_t item2;
    ASSERT_TRUE(stack_pop(s1, &item2));
}

// Peek

TEST_F(StackTestFxt, PeekNullStackReturnsFalse) {
    data_item_t item;
    ASSERT_FALSE(stack_peek(nullptr, &item));
}

TEST_F(StackTestFxt, PeekNonNullStackNullDestReturnsFalse) {
    ASSERT_FALSE(stack_peek(s1, nullptr));
}

TEST_F(StackTestFxt, PeekNonNullEmptyStackNonNullDestReturnsFalse) {
    data_item_t item;
    ASSERT_FALSE(stack_peek(s1, &item));
}

TEST_F(StackTestFxt, PeekReturnsTopItem) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 7};
    data_item_t item3 {.value = 42};
    data_item_t item4;
    ASSERT_TRUE(stack_push(s1, item1));
    ASSERT_TRUE(stack_push(s1, item2));
    ASSERT_TRUE(stack_push(s1, item3));
    ASSERT_TRUE(stack_peek(s1, &item4));
    ASSERT_EQ(item4.value, item3.value);
}

// Size

TEST_F(StackTestFxt, SizeNullStackIsZero) {
    ASSERT_EQ(stack_size(nullptr), 0);
}

TEST_F(StackTestFxt, SizeNonNullEmptyStackIsZero) {
    ASSERT_EQ(stack_size(s1), 0);
}

TEST_F(StackTestFxt, SizeNonNullNonEmptyStackMatches) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(stack_push(s1, item1));
    ASSERT_TRUE(stack_push(s1, item2));
    ASSERT_TRUE(stack_push(s1, item3));
    ASSERT_EQ(stack_size(s1), 3);
}

// Is empty

TEST_F(StackTestFxt, NullStackIsEmpty) {
    ASSERT_TRUE(stack_is_empty(nullptr));
}

TEST_F(StackTestFxt, NonNullZeroLengthStackIsEmpty) {
    data_item_t item;
    while (stack_pop(s1, &item));
    ASSERT_TRUE(stack_is_empty(s1));
}

TEST_F(StackTestFxt, IsCreatedEmpty) {
    ASSERT_TRUE(stack_is_empty(s1));
}

TEST_F(StackTestFxt, NonNullNonZeroLengthStackNotEmpty) {
    data_item_t item {.value = 1};
    ASSERT_TRUE(stack_push(s1, item));
    ASSERT_FALSE(stack_is_empty(s1));
}

// Destroy items

TEST_F(StackTestFxt, DestroyItemsTrueCallsItemDestructor) {
    otter_stack_t *s4 = stack_create();
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(stack_push(s4, item1));
    ASSERT_TRUE(stack_push(s4, item2));
    ASSERT_TRUE(stack_push(s4, item3));
    stack_destroy(s4, true, &mock_data_destructor);
    ASSERT_EQ(count_destructor_calls, 3);
}

TEST_F(StackTestFxt, DestroyItemsFalseDoesntCallItemDestructor) {
    otter_stack_t *s4 = stack_create();
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(stack_push(s4, item1));
    ASSERT_TRUE(stack_push(s4, item2));
    ASSERT_TRUE(stack_push(s4, item3));
    stack_destroy(s4, false, &mock_data_destructor);
    ASSERT_EQ(count_destructor_calls, 0);
}

// Item Order

TEST_F(StackTestFxt, ItemsReturnedLIFO) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(stack_push(s1, item1));
    ASSERT_TRUE(stack_push(s1, item2));
    ASSERT_TRUE(stack_push(s1, item3));
    ASSERT_TRUE(stack_pop(s1, &item4));
    ASSERT_EQ(item4.value, 3);
    ASSERT_TRUE(stack_pop(s1, &item4));
    ASSERT_EQ(item4.value, 2);
    ASSERT_TRUE(stack_pop(s1, &item4));
    ASSERT_EQ(item4.value, 1);
}

// Transfer Items

TEST_F(StackTestFxt, TransferToNullStackIsFalse) {
    ASSERT_FALSE(stack_transfer(nullptr, s1));
}

TEST_F(StackTestFxt, TransferToNonNullStackFromNullStackIsTrue) {
    ASSERT_TRUE(stack_transfer(s1, nullptr));
}

TEST_F(StackTestFxt, TransferToNonNullStackFromNonNullEmptyStackIsTrue) {
    ASSERT_TRUE(stack_is_empty(s2));
    ASSERT_TRUE(stack_transfer(s1, s2));
}

TEST_F(StackTestFxt, TransferToNonNullStackFromNonNullNonEmptyStackIsTrue) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(stack_push(s2, item1));
    ASSERT_TRUE(stack_push(s2, item2));
    ASSERT_TRUE(stack_push(s2, item3));
    ASSERT_TRUE(stack_push(s1, item4));
    ASSERT_TRUE(stack_transfer(s1, s2));
}

TEST_F(StackTestFxt, TransferSrcIsEmptyAfter) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(stack_push(s2, item1));
    ASSERT_TRUE(stack_push(s2, item2));
    ASSERT_TRUE(stack_push(s2, item3));
    ASSERT_TRUE(stack_push(s1, item4));
    ASSERT_FALSE(stack_is_empty(s2));
    ASSERT_TRUE(stack_transfer(s1, s2));
    ASSERT_TRUE(stack_is_empty(s2));
}

TEST_F(StackTestFxt, TransferDestNonEmptyAfter) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(stack_push(s2, item1));
    ASSERT_TRUE(stack_push(s2, item2));
    ASSERT_TRUE(stack_push(s2, item3));
    ASSERT_TRUE(stack_is_empty(s1));
    ASSERT_TRUE(stack_transfer(s1, s2));
    ASSERT_FALSE(stack_is_empty(s1));
}

TEST_F(StackTestFxt, TransferTotalStackSizeConserved) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(stack_push(s2, item1));
    ASSERT_TRUE(stack_push(s2, item2));
    ASSERT_TRUE(stack_push(s2, item3));
    ASSERT_TRUE(stack_push(s1, item4));
    std::size_t size1 = stack_size(s1), size2 = stack_size(s2);
    ASSERT_TRUE(stack_transfer(s1, s2));
    ASSERT_EQ(stack_size(s1), size1 + size2);
}