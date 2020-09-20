
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// test.cpp for SimpleXX/SimpleRenderer.

#include "vector"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "assert.h"
#include "vector.hpp"
#include "geometry.h"
#include "matrix.hpp"
#include "test.h"

using namespace std;

Test::Test(void) {
    return;
}

Test::~Test(void) {
    return;
}

bool Test::test_vector(void) const {
    Vector<int, 2> test(vector<int>{2, 3});
    Vector<int, 2> test2(vector<int>{-1, 3});
    Vector<int, 2> test3 = test - test2;
    assert(test.coord.x == 2);
    assert(test.coord.y == 3);
    assert(test2.coord.x == -1);
    assert(test2.coord.y == 3);
    assert(test3.coord.x == 3);
    assert(test3.coord.y == 0);
    assert((test3 * 3).coord.x == 9);
    assert((test3 * 3).coord.y == 0);
    assert((Vector<int, 2>(vector<int>{0, 1}) *
            Vector<int, 2>(vector<int>{1, 0})) == 0);
    assert(test.norm() == sqrt(13));
    assert((-test).coord.x == -2);
    assert((-test).coord.y == -3);
    assert(test == true);
    Vector<int, 2> test4;
    assert(test4 == false);
    Vector<int, 3> test5(vector<int>{2, 3, 4});
    Vector<int, 3> test6(vector<int>{-1, 3, -2});
    Vector<int, 3> test7 = test5 - test6;
    assert(test5.coord.x == 2);
    assert(test5.coord.y == 3);
    assert(test5.coord.z == 4);
    assert(test6.coord.x == -1);
    assert(test6.coord.y == 3);
    assert(test6.coord.z == -2);
    assert(test7.coord.x == 3);
    assert(test7.coord.y == 0);
    assert(test7.coord.z == 6);
    assert((test7 * 3).coord.x == 9);
    assert((test7 * 3).coord.y == 0);
    assert((test7 * 3).coord.z == 18);
    assert((Vector<int, 3>(vector<int>{0, 1, 0}) *
            Vector<int, 3>(vector<int>{1, 0, 0})) == 0);
    assert(test5.norm() == sqrt(29));
    assert((-test5).coord.x == -2);
    assert((-test5).coord.y == -3);
    assert((-test5).coord.z == -4);
    assert(test5 == true);
    Vector<int, 3> test8;
    assert(test8 == false);
    Vector<int, 3> test9 = test5 ^ test6;
    assert(test9.coord.x == -18);
    assert(test9.coord.y == 0);
    assert(test9.coord.z == 9);
    int            tmp233[3] = {1, 2, 3};
    Vector<int, 3> test10(tmp233);
    assert(test10.coord.x == 1);
    assert(test10.coord.y == 2);
    assert(test10.coord.z == 3);
    Vector<int, 3> test11(test10);
    assert(test11.coord.x == 1);
    assert(test11.coord.y == 2);
    assert(test11.coord.z == 3);
    return true;
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
    // int            arr2[9] = {2, 3, 3, 3, 4, 2, -2, -2, 3};
    // Matrix<int>    test8(3, 3, arr2);
    // Matrix<double> test9    = test8.inverse();
    double arr3[16] = {233};
    // total                   = test9.to_arr(arr3);
    // assert(total == 9);
    // assert(arr3[0] == -16);
    // assert(arr3[3] == 13);
    // assert(arr3[5] == -5);
    // assert(arr3[7] == 2);
    // assert(arr3[8] == 1);
    int         arr4[9] = {2, 6, 4, 5, 7, 9};
    Matrix<int> test10(2, 3, arr4);
    // Matrix<double> test11 = test10.inverse();
    std::vector<std::vector<size_t>> p;
    Matrix<int>                      test11 = test10.PLU(p);
    cout << test10 << endl;
    cout << test11 << endl;
    // total                 = test11.to_arr(arr3);
    // assert(total == 9);
    // assert(arr3[0] == -16);
    // assert(arr3[3] == 13);
    // assert(arr3[5] == -5);
    // assert(arr3[7] == 2);
    // assert(arr3[8] == 1);

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
    int      a[] = {0, 0};
    int      b[] = {0, 540};
    int      c[] = {960, 0};
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
