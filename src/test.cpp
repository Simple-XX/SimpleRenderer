
// This file is a part of Simple-XX/SimpleRenderer
// (https://github.com/Simple-XX/SimpleRenderer).
//
// test.cpp for Simple-XX/SimpleRenderer.

#include "vector"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "assert.h"
#include "test.h"
#include "vector.hpp"
#include "geometry.h"
#include "matrix.hpp"

using namespace std;

Test::Test(void) {
    return;
}

Test::~Test(void) {
    return;
}

bool Test::test_matrix(void) const {
    Matrix<int> test1;
    int         arr[16] = {233};
    size_t      total   = test1.to_arr(arr);
    assert(total == 16);
    assert(arr[0] == 0);
    assert(arr[3] == 0);
    assert(arr[5] == 0);
    assert(arr[7] == 0);
    assert(arr[8] == 0);
    assert(arr[9] == 0);
    assert(arr[15] == 0);
    vector<vector<int>> tmp{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
    Matrix<int>         test2(tmp);
    total = test2.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 1);
    assert(arr[3] == 4);
    assert(arr[5] == 6);
    assert(arr[7] == 8);
    assert(arr[8] == 9);
    assert(arr[9] == 10);
    assert(arr[11] == 12);
    Matrix<int> test3 = test2.transpose();
    total             = test3.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 1);
    assert(arr[3] == 2);
    assert(arr[5] == 10);
    assert(arr[7] == 7);
    assert(arr[8] == 11);
    assert(arr[9] == 4);
    assert(arr[11] == 12);
    int         arr1[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    Matrix<int> test4(3, 4, arr1);
    total = test4.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 1);
    assert(arr[3] == 4);
    assert(arr[5] == 6);
    assert(arr[7] == 8);
    assert(arr[8] == 9);
    assert(arr[9] == 10);
    assert(arr[11] == 12);
    Matrix<int> test5 = test4.transpose();
    total             = test5.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 1);
    assert(arr[3] == 2);
    assert(arr[5] == 10);
    assert(arr[7] == 7);
    assert(arr[8] == 11);
    assert(arr[9] == 4);
    assert(arr[11] == 12);
    Matrix<int> test6(test2);
    total = test6.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 1);
    assert(arr[3] == 4);
    assert(arr[5] == 6);
    assert(arr[7] == 8);
    assert(arr[8] == 9);
    assert(arr[9] == 10);
    assert(arr[11] == 12);
    Matrix<int> test7 = test2 + test2;
    total             = test7.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 2);
    assert(arr[3] == 8);
    assert(arr[5] == 12);
    assert(arr[7] == 16);
    assert(arr[8] == 18);
    assert(arr[9] == 20);
    assert(arr[11] == 24);
    test7 = test2 - test2;
    total = test7.to_arr(arr);
    assert(total == 12);
    assert(arr[0] == 0);
    assert(arr[3] == 0);
    assert(arr[5] == 0);
    assert(arr[7] == 0);
    assert(arr[8] == 0);
    assert(arr[9] == 0);
    assert(arr[11] == 0);
    test7 = test2 * test3;
    total = test7.to_arr(arr);
    assert(total == 9);
    assert(arr[0] == 30);
    assert(arr[3] == 70);
    assert(arr[5] == 278);
    assert(arr[7] == 278);
    assert(arr[8] == 446);
    test7 -= test7;
    total = test7.to_arr(arr);
    assert(total == 9);
    assert(arr[0] == 0);
    assert(arr[3] == 0);
    assert(arr[5] == 0);
    assert(arr[7] == 0);
    assert(arr[8] == 0);
    test2 *= test3;
    total = test2.to_arr(arr);
    assert(total == 9);
    assert(arr[0] == 30);
    assert(arr[3] == 70);
    assert(arr[5] == 278);
    assert(arr[7] == 278);
    assert(arr[8] == 446);
    int           arr2[9] = {2, 3, 3, 3, 4, 2, -2, -2, 3};
    Matrix<int>   test8(3, 3, arr2);
    Matrix<float> test9    = test8.inverse();
    float         arr3[16] = {233};
    total                  = test9.to_arr(arr3);
    assert(total == 9);
    assert(arr3[0] == -16);
    assert(arr3[3] == 13);
    assert(arr3[5] == -5);
    assert(arr3[7] == 2);
    assert(arr3[8] == 1);
    int           arr4[16] = {1, 0, 0, 0, 2, 1, 0, 0, 3, 2, 1, 0, 4, 3, 2, 1};
    Matrix<int>   test10(4, 4, arr4);
    Matrix<float> test11 = test10.inverse();
    total                = test11.to_arr(arr3);
    assert(total == 16);
    assert(arr3[0] == 1);
    assert(arr3[3] == 0);
    assert(arr3[5] == 1);
    assert(arr3[7] == 0);
    assert(arr3[8] == 1);
    assert(arr3[15] == 1);
    assert(arr3[14] == -2);
    assert(arr3[12] == 0);
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
    size_t   a[] = {0, 0};
    size_t   b[] = {0, 540};
    size_t   c[] = {960, 0};
    Vectors2 A(a);
    Vectors2 B(b);
    Vectors2 C(c);
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
