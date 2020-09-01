
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// test.cpp for SimpleXX/SimpleRenderer.

#include "vector"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "assert.h"
#include "vector.hpp"
#include "geometry.h"
#include "test.h"

using namespace std;

Test::Test(void) {
    return;
}

Test::~Test(void) {
    return;
}

bool Test::test_vector(void) const {
    cout << "==========" << "Test Vector<int, 2>" << "==========" << endl;
    Vector<int, 2> test(vector<int>{ 2, 3 });
    Vector<int, 2> test2(vector<int>{ -1, 3 });
    Vector<int, 2> test3 = test - test2;
    assert(test.get_vect()[0] == 2);
    assert(test.get_vect()[1] == 3);
    assert(test2.get_vect()[0] == -1);
    assert(test2.get_vect()[1] == 3);
    assert(test3.get_vect()[0] == 3);
    assert(test3.get_vect()[1] == 0);
    assert( (test3 * 3).get_vect()[0] == 9);
    assert( (test3 * 3).get_vect()[1] == 0);
    assert( (Vector<int, 2>(vector<int>{ 0, 1 }) * Vector<int, 2>(vector<int>{ 1, 0 }) ) == 0);
    assert(test.get_norm() == sqrt(13) );
    assert( (-test).get_vect()[0] == -2);
    assert( (-test).get_vect()[1] == -3);
    assert(test == true);
    Vector<int, 2> test4;
    assert(test4 == false);
    cout << test << test2 << test3 << test4 << endl;
    cout << "==========" << "Test Vector<int, 3>" << "==========" << endl;
    Vector<int, 3> test5(vector<int>{ 2, 3, 4 });
    Vector<int, 3> test6(vector<int>{ -1, 3, -2 });
    Vector<int, 3> test7 = test5 - test6;
    assert(test5.get_vect()[0] == 2);
    assert(test5.get_vect()[1] == 3);
    assert(test5.get_vect()[2] == 4);
    assert(test6.get_vect()[0] == -1);
    assert(test6.get_vect()[1] == 3);
    assert(test6.get_vect()[2] == -2);
    assert(test7.get_vect()[0] == 3);
    assert(test7.get_vect()[1] == 0);
    assert(test7.get_vect()[2] == 6);
    assert( (test7 * 3).get_vect()[0] == 9);
    assert( (test7 * 3).get_vect()[1] == 0);
    assert( (test7 * 3).get_vect()[2] == 18);
    assert( (Vector<int, 3>(vector<int>{ 0, 1, 0 }) * Vector<int, 3>(vector<int>{ 1, 0, 0 }) ) == 0);
    assert(test5.get_norm() == sqrt(29) );
    assert( (-test5).get_vect()[0] == -2);
    assert( (-test5).get_vect()[1] == -3);
    assert( (-test5).get_vect()[2] == -4);
    assert(test5 == true);
    Vector<int, 3> test8;
    assert(test8 == false);
    Vector<int, 3> test9  = test5 ^ test6;
    assert(test9.get_vect()[0] == -18);
    assert(test9.get_vect()[1] == 0);
    assert(test9.get_vect()[2] == 9);
    // assert( ( (++test9).get_vect() ).at(0) == -17);
    // assert( (test9.get_vect() ).at(1) == 1);
    // assert( (test9.get_vect() ).at(2) == 10);
    // assert( ( (test9++).get_vect() ).at(0) == -17);
    // assert( (test9.get_vect() ).at(1) == 2);
    // assert( (test9.get_vect() ).at(2) == 11);
    cout << test5 << test6 << test7 << test8 << test9 << endl;
    return true;
}

bool Test::test_line(void) const {
    cout << "==========Test line==========" << endl;
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    // 左右对角线
    painter.line(0, 0, 1920, 1080, white);
    painter.line(0, 1080, 1920, 0, white);
    // 居中水平线
    painter.line(0, 540, 1920, 540, white);
    // 居中铅锤线
    painter.line(960, 0, 960, 1080, white);
    painter.save("test_line.tga");
    return true;
}

bool Test::test_triangle(void) const {
    cout << "==========Test triangle==========" << endl;
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    painter.line(0, 0, 100, 100, white);
    int a[] = { 0, 0 };
    int b[] = { 0, 540 };
    int c[] = { 960, 0 };
    Vectori2 A(a);
    Vectori2 B(b);
    Vectori2 C(c);
    painter.triangle(A, B, C, white);
    painter.save("test_triangle.tga");
    return true;
}

bool Test::test_fill(void) const {
    cout << "==========Test fill==========" << endl;
    TGAImage image(width, height, TGAImage::RGBA);
    Geometry painter(image);
    painter.line(0, 0, 100, 100, white);
    painter.save("test_triangle.tga");
    return true;
}

bool Test::test_2d(void) const {
    test_line();
    test_triangle();
    // test_fill();
    return true;
}
