#include <gtest/gtest.h>

TEST(my, mycase) {
    ASSERT_EQ(12, 12);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}