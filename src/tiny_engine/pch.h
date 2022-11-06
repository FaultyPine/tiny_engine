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
#include <map>
#include <math.h>
#include <algorithm>
#include <memory>
// ----------------

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
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
        std::cout << "OpenGL error: (" << error << "): " << file << " line: " << line << std::endl;
        return false;
    }
    return true;
}

#ifdef TINY_DEBUG
    #define GLCall(x) GLClearError();\
        x;\
        ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#else
    #define GLCall(x) x
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



inline std::string Vec3ToStr(const glm::vec3& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
inline std::string Vec2ToStr(const glm::vec2& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y);
}

#endif