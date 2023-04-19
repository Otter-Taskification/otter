#include <gtest/gtest.h>
#include "public/types/queue.h"

void mock_data_destructor(void *ptr);

static int count_destructor_calls;

namespace {
class QueueTestFxt : public testing::Test {
    protected:

    otter_queue_t *q1;
    otter_queue_t *q2;
    otter_queue_t *q3;

    void SetUp() override {
        q1 = queue_create();
        q2 = queue_create();
        q3 = queue_create();
        count_destructor_calls = 0;
    }

    virtual void TearDown() override {
        queue_destroy(q1, false, nullptr);
        queue_destroy(q2, false, nullptr);
        queue_destroy(q3, false, nullptr);
        count_destructor_calls = 0;
    }

};
}

// Helper functions
void mock_data_destructor(void *ptr) {
    count_destructor_calls++;
}

TEST_F(QueueTestFxt, IsNonNull) {
    ASSERT_NE(q1, nullptr);
    ASSERT_NE(q2, nullptr);
    ASSERT_NE(q3, nullptr);
}

// Push

TEST_F(QueueTestFxt, PushNullQueueIsFalse) {
    data_item_t item {.value = 1};
    ASSERT_FALSE(queue_push(nullptr, item));
}

TEST_F(QueueTestFxt, PushNonNullQueueIsTrue) {
    data_item_t item {.value = 1};
    ASSERT_TRUE(queue_push(q1, item));
}

// Pop

TEST_F(QueueTestFxt, PopNullQueueIsFalse) {
    data_item_t item2;
    ASSERT_FALSE(queue_pop(nullptr, &item2));
}

TEST_F(QueueTestFxt, PopNonNullEmptyQueueIsFalse) {
    data_item_t item2;
    ASSERT_FALSE(queue_pop(q1, &item2));
}

TEST_F(QueueTestFxt, PopNonNullNonEmptyQueueIsTrue) {
    data_item_t item1 {.value = 1};
    ASSERT_TRUE(queue_push(q1, item1));
    data_item_t item2;
    ASSERT_TRUE(queue_pop(q1, &item2));
}

// Length

TEST_F(QueueTestFxt, LengthNullQueueIsZero) {
    ASSERT_EQ(queue_length(nullptr), 0);
}

TEST_F(QueueTestFxt, LengthNonNullEmptyQueueIsZero) {
    ASSERT_EQ(queue_length(q1), 0);
}

TEST_F(QueueTestFxt, LengthNonNullNonEmptyQueueMatches) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(queue_push(q1, item1));
    ASSERT_TRUE(queue_push(q1, item2));
    ASSERT_TRUE(queue_push(q1, item3));
    ASSERT_EQ(queue_length(q1), 3);
}

// Is empty

TEST_F(QueueTestFxt, NullQueueIsEmpty) {
    ASSERT_TRUE(queue_is_empty(nullptr));
}

TEST_F(QueueTestFxt, NonNullZeroLengthQueueIsEmpty) {
    data_item_t item;
    while (queue_pop(q1, &item));
    ASSERT_TRUE(queue_is_empty(q1));
}

TEST_F(QueueTestFxt, IsCreatedEmpty) {
    ASSERT_TRUE(queue_is_empty(q1));
}

TEST_F(QueueTestFxt, NonNullNonZeroLengthQueueNotEmpty) {
    data_item_t item {.value = 1};
    ASSERT_TRUE(queue_push(q1, item));
    ASSERT_FALSE(queue_is_empty(q1));
}

// Destroy items

TEST_F(QueueTestFxt, DestroyItemsTrueCallsItemDestructor) {
    otter_queue_t *q4 = queue_create();
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(queue_push(q4, item1));
    ASSERT_TRUE(queue_push(q4, item2));
    ASSERT_TRUE(queue_push(q4, item3));
    queue_destroy(q4, true, &mock_data_destructor);
    ASSERT_EQ(count_destructor_calls, 3);
}

TEST_F(QueueTestFxt, DestroyItemsFalseDoesntCallItemDestructor) {
    otter_queue_t *q4 = queue_create();
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(queue_push(q4, item1));
    ASSERT_TRUE(queue_push(q4, item2));
    ASSERT_TRUE(queue_push(q4, item3));
    queue_destroy(q4, false, &mock_data_destructor);
    ASSERT_EQ(count_destructor_calls, 0);
}

// Append

TEST_F(QueueTestFxt, AppendToFromNullQueueIsFalse) {
    ASSERT_FALSE(queue_append(q1, nullptr));
    ASSERT_FALSE(queue_append(nullptr, q1));
}

TEST_F(QueueTestFxt, AppendFromEmptyQueueIsTrue) {
    ASSERT_TRUE(queue_append(q1, q2));
}

TEST_F(QueueTestFxt, AppendToNullFromNonEmptyQueueIsFalse) {
    data_item_t item1 {.value = 1};
    ASSERT_TRUE(queue_push(q2, item1));
    ASSERT_FALSE(queue_append(nullptr, q2));
}

TEST_F(QueueTestFxt, LengthOfDestAfterAppendMatches) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(queue_push(q1, item1));
    ASSERT_TRUE(queue_push(q1, item2));
    ASSERT_TRUE(queue_push(q1, item3));
    size_t length_before = queue_length(q1);
    ASSERT_TRUE(queue_append(q2, q1));
    ASSERT_EQ(queue_length(q2), length_before);
}

TEST_F(QueueTestFxt, LengthOfSrcAfterAppendIsZero) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    ASSERT_TRUE(queue_push(q1, item1));
    ASSERT_TRUE(queue_push(q1, item2));
    ASSERT_TRUE(queue_push(q1, item3));
    ASSERT_TRUE(queue_append(q2, q1));
    ASSERT_EQ(queue_length(q1), 0);
}

// Item Order

TEST_F(QueueTestFxt, ItemsReturnedFIFO) {
    data_item_t item1 {.value = 1};
    data_item_t item2 {.value = 2};
    data_item_t item3 {.value = 3};
    data_item_t item4 {.value = 0};
    ASSERT_TRUE(queue_push(q1, item1));
    ASSERT_TRUE(queue_push(q1, item2));
    ASSERT_TRUE(queue_push(q1, item3));
    ASSERT_TRUE(queue_pop(q1, &item4));
    ASSERT_EQ(item4.value, 1);
    ASSERT_TRUE(queue_pop(q1, &item4));
    ASSERT_EQ(item4.value, 2);
    ASSERT_TRUE(queue_pop(q1, &item4));
    ASSERT_EQ(item4.value, 3);
}
