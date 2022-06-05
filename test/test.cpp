
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// test.cpp for Simple-XX/SimpleRenderer.

#include "gtest/gtest.h"

#include "vector.hpp"

TEST(Vector2f, test1) {
    Vector2i test(2, 3);
    Vector2i test2(-1, 3);
    Vector2i test3 = test - test2;
    assert(test.x == 2);
    assert(test.y == 3);
    assert(test2.x == -1);
    assert(test2.y == 3);
    assert(test3.x == 3);
    assert(test3.y == 0);
    assert((test3 * 3).x == 9);
    assert((test3 * 3).y == 0);
    // assert((Vector2i(0, 1) * Vector2i(1, 0)) == 0);
    Vector2i a(Dot(Vector2i(0, 1), Vector2i(1, 0)));
    // assert(test.norm() == sqrt(13));
    // assert((-test).x == -2);
    // assert((-test).y == -3);
    // assert(test == true);
    // Vector2i test4;
    // assert(test4 == false);
    // Vector3i test5(2, 3, 4);
    // Vector3i test6(-1, 3, -2);
    // Vector3i test7 = test5 - test6;
    // assert(test5.x == 2);
    // assert(test5.y == 3);
    // assert(test5.z == 4);
    // assert(test6.x == -1);
    // assert(test6.y == 3);
    // assert(test6.z == -2);
    // assert(test7.x == 3);
    // assert(test7.y == 0);
    // assert(test7.z == 6);
    // assert((test7 * 3).x == 9);
    // assert((test7 * 3).y == 0);
    // assert((test7 * 3).z == 18);
    // assert((Vector3i(0, 1, 0) * Vector3i(1, 0, 0)) == 0);
    // assert(test5.norm() == sqrt(29));
    // assert((-test5).x == -2);
    // assert((-test5).y == -3);
    // assert((-test5).z == -4);
    // assert(test5 == true);
    // Vector3i test8;
    // assert(test8 == false);
    // Vector3i test9 = test5 ^ test6;
    // assert(test9.x == -18);
    // assert(test9.y == 0);
    // assert(test9.z == 9);
    // int            tmp233[3] = {1, 2, 3};
    // Vector3i test10(tmp233);
    // assert(test10.x == 1);
    // assert(test10.y == 2);
    // assert(test10.z == 3);
    // Vector3i test11(test10);
    // assert(test11.x == 1);
    // assert(test11.y == 2);
    // assert(test11.z == 3);
    return;
}
