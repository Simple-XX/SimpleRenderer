
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// test.h for SimpleXX/SimpleRenderer.

#ifndef __TEST_H__
#define __TEST_H__

#include "image.h"

class Test {
private:
    const TGAColor white  = TGAColor(255, 255, 255, 255);
    const TGAColor red    = TGAColor(255, 0, 0, 255);
    const int      width  = 1920;
    const int      height = 1080;
    bool           test_line(void) const;
    bool           test_triangle(void) const;
    bool           test_fill(void) const;

public:
    Test(void);
    ~Test(void);
    bool test_vector(void) const;
    bool test_matrix(void) const;
    bool test_2d() const;
    bool test_tga(void) const;
};

#endif /* __TEST_H__ */
