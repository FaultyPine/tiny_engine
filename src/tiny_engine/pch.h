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
// ----------------
#define PATH_MAX 260
#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define TINY_DEBUG
#define TARGET_FPS 60

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
inline void PrintAndExitDBG(const char* file, s32 line, const char* msg) {
    std::cout << "[ERROR] In " << file << " on line " << line << "\n" << msg << std::endl; 
    exit(1);
}
#ifdef TINY_DEBUG
#define ASSERT(x) if (!(x)) PrintAndExitDBG(__FILE__, __LINE__, #x)
#else
#define ASSERT(x)
#endif

// OpenGL error handling
static void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}
static bool GLLogCall(const char* func, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "OpenGL error: (" << error << "): " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

#ifdef TINY_DEBUG
#define GLCall(_CALL)      do { _CALL; GLenum gl_err = glGetError(); if (gl_err != 0) fprintf(stderr, "GL error 0x%x returned from '%s' %s:%i.\n", gl_err, #_CALL, __FILE__, __LINE__); } while (0)
#else
#define GLCall(_CALL)      _CALL   // Call without error check
#endif

const double PI  =3.141592653589793238463;
const float  PI_F=3.14159265358979f;

// Utils
template<typename T> inline T MAX(T x, T y) { return x > y ? x : y; }
template<typename T> inline T MIN(T x, T y) { return x < y ? x : y; }

#define ARRAY_SIZE(arr) ( sizeof((arr))/sizeof((arr)[0]) )

template <typename T> void CLAMP(T& value, const T& low, const T& high) {
    value = value < low ? low : (value > high ? high : value); 
}

template <typename T> int signof(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T> bool isInRange(T x, T min, T max) {
    return min <= x && x <= max;
}

inline uint32_t hash(const char* message, size_t message_length)
{
    auto mix = [](uint32_t message_block, uint32_t internal_state){
        return (internal_state * message_block) ^
          ((internal_state << 3) + (message_block >> 2));
    };

   uint32_t internal_state = 0xA5A5A5A5; // IV: A magic number
   uint32_t message_block = 0;

   // Loop over the message 32-bits at-a-time
   while (message_length >= 4)
   {
      memcpy(&message_block, message, sizeof(uint32_t));

      internal_state = mix(message_block, internal_state);

      message_length -= sizeof(uint32_t);
      message        += sizeof(uint32_t);
   }

   // Are there any remaining bytes?
   if (message_length)
   {
      memcpy(&message_block, message, message_length);
      internal_state = mix(message_block, internal_state);
   }

   return internal_state;
}

inline u32 countLeadingZeroes(u32 n) {
    return n == 0 ? 0 : log2(n & -n);
}

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

inline std::string VecToStr(const glm::vec2& vec) {
    return std::string(TextFormat("%f %f", vec.x, vec.y));
}
inline std::string VecToStr(const glm::vec3& vec) {
    return std::string(TextFormat("%f %f %f", vec.x, vec.y, vec.z));
}
inline std::string VecToStr(const glm::vec4& vec) {
    return std::string(TextFormat("%f %f %f %f", vec.x, vec.y, vec.z, vec.w));
}
inline std::string Matrix4x4ToStr(const glm::mat4 mat) {
    return glm::to_string(mat);
}

#endif