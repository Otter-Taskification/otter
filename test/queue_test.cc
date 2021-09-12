#include <gtest/gtest.h>
#include "otter/queue.h"

// TEST(QueueTest, MakeQueue) {

//     otter_queue_t *queue = queue_create();

//     // Must not be NULL
//     ASSERT_NE(queue, nullptr);
// }

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
    }

    virtual void TearDown() override {
        queue_destroy(q1, false, nullptr);
        queue_destroy(q2, false, nullptr);
        queue_destroy(q3, false, nullptr);
    }

};
}

TEST_F(QueueTestFxt, MakeQueue) {
    ASSERT_NE(q1, nullptr);
    ASSERT_NE(q2, nullptr);
    ASSERT_NE(q3, nullptr);
}
