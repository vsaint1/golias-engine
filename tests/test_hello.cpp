#include <gtest/gtest.h>

int add(int a, int b) {
    return a + b;
}
TEST(BasicSetup, Setup) {

    EXPECT_EQ(add(1, 2), 3);
}
