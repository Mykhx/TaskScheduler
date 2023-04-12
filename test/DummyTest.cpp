#include <gtest/gtest.h>

TEST(testA, test1) {
    EXPECT_EQ (0, 0);
}

TEST(testA, test2) {
    EXPECT_EQ (2, 2);
    EXPECT_EQ (4, 4);
}

TEST(testA, test3) {
    EXPECT_EQ (0, 0);
    EXPECT_EQ (2, 3);
    EXPECT_EQ (4, 4);
}