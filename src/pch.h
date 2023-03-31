#ifndef TINY_PCH_H
#define TINY_PCH_H

// STD
#include <string>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include <random>
#include <map>
#include <math.h>
#include <algorithm>
#include <memory>
#include <stdarg.h>
#include <optional>
// ----------------
#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



// types
typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long u64;
typedef long s64;
typedef float f32;
typedef double f64;
typedef unsigned long long usize;
typedef signed long long isize;
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

// debug
inline void PrintAndExitDBG(const char* file, s32 line, const char* msg) {
    std::cout << "[ERROR] In " << file << ":" << line << "\n" << msg << std::endl; 
    throw std::exception();
}
#ifdef TINY_DEBUG
#include <intrin.h>
#define ASSERT(x) if (!(x)) PrintAndExitDBG(__FILE__, __LINE__, #x)
#else
#define ASSERT(x)
#endif

// OpenGL error handling
static bool GLLogCall(const char* func, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL error: (" << error << "): " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

#ifdef TINY_DEBUG
#define GLCall(_CALL)  \
    do { \
        _CALL; GLenum gl_err = glGetError(); \
        if (gl_err != 0) { \
            fprintf(stderr, "GL error 0x%x returned from '%s' %s:%i.\n", gl_err, #_CALL, __FILE__, __LINE__); \
            throw std::exception(); \
        } \
    } while (0)
#else
#define GLCall(_CALL)      _CALL   // Call without error check
#endif

#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )

// yoinked from raylib
// https://github.com/raysan5/raylib/blob/master/src/rcore.c#L7169
inline const char *TextFormat(const char *text, ...)
{
#ifndef MAX_TEXTFORMAT_BUFFERS
    #define MAX_TEXTFORMAT_BUFFERS      12        // Maximum number of static buffers for text formatting
#endif
#ifndef MAX_TEXT_BUFFER_LENGTH
    #define MAX_TEXT_BUFFER_LENGTH   1024        // Maximum size of static text buffer
#endif

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}


#endif