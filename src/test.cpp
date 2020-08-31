
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// test.cpp for SimpleXX/SimpleRenderer.

#include "vector"
#include "iostream"
#include "math.h"
#include "stdarg.h"
#include "assert.h"
#include "vector.hpp"
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
    assert( (test.get_vect() ).at(0) == 2);
    assert( (test.get_vect() ).at(1) == 3);
    assert( (test2.get_vect() ).at(0) == -1);
    assert( (test2.get_vect() ).at(1) == 3);
    assert( (test3.get_vect() ).at(0) == 3);
    assert( (test3.get_vect() ).at(1) == 0);
    assert( ( (test3 * 3).get_vect() ).at(0) == 9);
    assert( ( (test3 * 3).get_vect() ).at(1) == 0);
    assert( (Vector<int, 2>(vector<int>{ 0, 1 }) * Vector<int, 2>(vector<int>{ 1, 0 }) ) == 0);
    assert(test.get_norm() == sqrt(13) );
    assert( ( (-test).get_vect() ).at(0) == -2);
    assert( ( (-test).get_vect() ).at(1) == -3);
    assert(test == true);
    Vector<int, 2> test4;
    assert(test4 == false);
    cout << test << test2 << test3 << test4 << endl;
    cout << "==========" << "Test Vector<int, 3>" << "==========" << endl;
    Vector<int, 3> test5(vector<int>{ 2, 3, 4 });
    Vector<int, 3> test6(vector<int>{ -1, 3, -2 });
    Vector<int, 3> test7 = test5 - test6;
    assert( (test5.get_vect() ).at(0) == 2);
    assert( (test5.get_vect() ).at(1) == 3);
    assert( (test5.get_vect() ).at(2) == 4);
    assert( (test6.get_vect() ).at(0) == -1);
    assert( (test6.get_vect() ).at(1) == 3);
    assert( (test6.get_vect() ).at(2) == -2);
    assert( (test7.get_vect() ).at(0) == 3);
    assert( (test7.get_vect() ).at(1) == 0);
    assert( (test7.get_vect() ).at(2) == 6);
    assert( ( (test7 * 3).get_vect() ).at(0) == 9);
    assert( ( (test7 * 3).get_vect() ).at(1) == 0);
    assert( ( (test7 * 3).get_vect() ).at(2) == 18);
    assert( (Vector<int, 3>(vector<int>{ 0, 1, 0 }) * Vector<int, 3>(vector<int>{ 1, 0, 0 }) ) == 0);
    assert(test5.get_norm() == pow(29, (1.0 / 3) ) );
    assert( ( (-test5).get_vect() ).at(0) == -2);
    assert( ( (-test5).get_vect() ).at(1) == -3);
    assert( ( (-test5).get_vect() ).at(2) == -4);
    assert(test5 == true);
    Vector<int, 3> test8;
    assert(test8 == false);
    Vector<int, 3> test9  = test5 ^ test6;
    assert( (test9.get_vect() ).at(0) == -18);
    assert( (test9.get_vect() ).at(1) == 0);
    assert( (test9.get_vect() ).at(2) == 9);
    cout << test5 << test6 << test7 << test8 << test9 << endl;
    return true;
}
