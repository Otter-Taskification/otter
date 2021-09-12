#include <gtest/gtest.h>
#include "otter/stack.h"

// TEST(StackTest, MakeStack) {

//     otter_stack_t *stack = stack_create();

//     // Must not be NULL
//     ASSERT_NE(stack, nullptr);
// }

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
    }

    virtual void TearDown() override {
        stack_destroy(s1, false, nullptr);
        stack_destroy(s2, false, nullptr);
        stack_destroy(s3, false, nullptr);
    }

};
}

TEST_F(StackTestFxt, MakeStack) {
    ASSERT_NE(s1, nullptr);
    ASSERT_NE(s2, nullptr);
    ASSERT_NE(s3, nullptr);
}

