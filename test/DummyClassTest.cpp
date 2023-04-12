#include <gtest/gtest.h>
#include "DummyClass.h"

TEST(DummyClassTest, test1) {
    EXPECT_TRUE(DummyClass::canBeInvoked());
}