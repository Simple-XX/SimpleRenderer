
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// test.cpp for Simple-XX/SimpleRenderer.

#include "gtest/gtest.h"

#include "vector.hpp"

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
    // Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
    // Expect equality.
    EXPECT_EQ(7 * 6, 42);
}

// Demonstrate some basic assertions.
TEST(Vector2f, test1) {
    Vector2f a(2, 3);
    Vector2f b(2, 3);
    Vector2f c = a + b;
    EXPECT_EQ(c.x, 4);
    EXPECT_EQ(c.y, 6);
    return;
}
