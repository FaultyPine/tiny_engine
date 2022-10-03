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
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include <random>
// ----------------

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLT_DEBUG
#define GLT_DEBUG_PRINT
#define GLT_IMPLEMENTATION
// because this is defined, make sure to call gltViewport(width, height) when the screen is resized
// this optimizes away a call to glGetIntegerv
#define GLT_MANUAL_VIEWPORT 
#include "gltext.h"

//#include "stb_image.h"

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

// debug
inline void PrintAndExitDBG(const char* file, s32 line) {
    std::cout << "[ERROR] In " << file << " on line " << line << std::endl; 
    exit(1);
}
#define ASSERT(x) if (!(x)) PrintAndExitDBG(__FILE__, __LINE__)

// OpenGL error handling
static void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}
static bool GLLogCall(const char* func, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL error: (" << error << "): " << file << " line: " << line << std::endl;
        return false;
    }
    return true;
}
/*
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
*/
#define GLCall(x) x


// Utils
template<typename T> inline T MAX(T x, T y) { return x > y ? x : y; }
template<typename T> inline T MIN(T x, T y) { return x < y ? x : y; }

#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )

template <typename T> void CLAMP(T& value, const T& low, const T& high) {
    value = value < low ? low : (value > high ? high : value); 
}


#endif