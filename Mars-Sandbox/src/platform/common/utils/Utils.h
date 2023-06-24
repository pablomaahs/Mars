#pragma once

#include <string.h>

#define ASSERT(value) CHECK(value, __FILE__, __LINE__)

static void CHECK(bool check, const char* fileName, int lineNumber)
{
    if (!check)
    {
        printf("CHECK() failed at %s:%i\n", fileName, lineNumber);
        __debugbreak();
        exit(EXIT_FAILURE);
    }
}

int endsWith(const char* s, const char* part);

std::string readShaderFile(const char* fileName);

void printShaderSource(const char* text);