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



#define PRINT_AND_EXIT() std::cout << "[ERROR] In " << __FILE__ << " on line " << __LINE__ << std::endl; exit(1)

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
#define ASSERT(x) if (!(x)) PRINT_AND_EXIT()

/*
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
*/
#define GLCall(x) x

template<typename T> inline T MAX(T x, T y) { return x > y ? x : y; }
template<typename T> inline T MIN(T x, T y) { return x < y ? x : y; }
#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )

template <typename T> T CLAMP(const T& value, const T& low, const T& high) {
    return value < low ? low : (value > high ? high : value); 
}

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


#endif