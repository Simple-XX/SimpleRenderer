
// This file is a part of SimpleXX/SimpleRenderer (https://github.com/SimpleXX/SimpleRenderer).
//
// test.h for SimpleXX/SimpleRenderer.

#ifndef __TEST_H__
#define __TEST_H__

class Test {
private:
    bool test_line(void) const;

public:
    Test(void);
    ~Test(void);
    bool test_vector(void) const;
    bool test_2d() const;
    bool test_tga(void) const;
};

#endif /* __TEST_H__ */
