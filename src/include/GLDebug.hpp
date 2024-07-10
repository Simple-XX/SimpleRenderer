#pragma once

#define ASSERT(x) if(!(x)) __builtin_trap();

#define GLCall(x) GLCleanError(); \
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLCleanError();
bool GLLogCall(const char* function, const char* file, int line);