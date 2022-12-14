#ifndef TINY_MATH_H
#define TINY_MATH_H

#include "pch.h"

const double PI   = 3.141592653589793238463;
const float  PI_F = 3.14159265358979f;

namespace Math {



bool isOverlappingRect2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2);
bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist);

f32 Lerp(f32 a, f32 b, f32 t);
f32 InvLerp(f32 a, f32 b, f32 v);
f32 Remap(f32 iMin, f32 iMax, f32 oMin, f32 oMax, f32 v);



template<typename T> inline T MAX(T x, T y) { return x > y ? x : y; }
template<typename T> inline T MIN(T x, T y) { return x < y ? x : y; }
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

inline glm::mat4 Position3DToModelMat(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1), f32 rotation = 0.0, const glm::vec3& rotationAxis = {1,0,0}) {
    glm::mat4 model = glm::mat4(1);
    model = glm::rotate(model, glm::radians(rotation), rotationAxis); 
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));
    return model;
}


}



#endif