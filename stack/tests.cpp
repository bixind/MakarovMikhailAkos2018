#include <gtest/gtest.h>
#include "stack.h"

TEST(StackBasic, CallTest) {
    Stack<int> s;
    ASSERT_TRUE(s.Empty());
    for (int i = 1; i < 10; ++i) {
        s.Push(i);
    }
    ASSERT_EQ(s.Size(), 9u);
    for (int i = 9; i > 0; --i) {
        int r = -1;
        ASSERT_TRUE(s.Pop(&r));
        ASSERT_EQ(r, i);
    }
    int r = 0;
    ASSERT_FALSE(s.Pop(&r));
}

TEST(StackBasic, BigTest) {
    Stack<int> s;
    int n = 10000;
    for (int i = 0; i < n; ++i) {
        s.Push(i);
    }
    for (int i = n - 1; i >= 0; --i) {
        int r = -1;
        ASSERT_TRUE(s.Pop(&r));
        ASSERT_EQ(r, i);
    }
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}