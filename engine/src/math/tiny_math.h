#ifndef TINY_MATH_H
#define TINY_MATH_H

//#include "pch.h"
#include "tiny_defines.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr double PI   = 3.141592653589793238463;
constexpr float  PI_F = 3.14159265358979f;

inline f32 DegToRad(f32 deg)
{
    return deg * (PI / 180);
}
inline f32 RadToDeg(f32 rad)
{
    return rad * (180/PI);
}

struct Frustum
{
    glm::vec4 nearTopLeft;
    glm::vec4 nearBottomLeft;
    glm::vec4 nearTopRight;
    glm::vec4 nearBottomRight;
    glm::vec4 farTopLeft;
    glm::vec4 farBottomLeft;
    glm::vec4 farTopRight;
    glm::vec4 farBottomRight;
};

namespace Math {



TAPI bool isOverlappingRectSize2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2);
TAPI bool isOverlappingRect2D(const glm::vec2& startPos1, const glm::vec2& endPos1, const glm::vec2& startPos2, const glm::vec2& endPos2);
TAPI bool isPointInRectangle(const glm::vec2& point, const glm::vec2& rectStart, const glm::vec2& rectEnd);
TAPI bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist);

TAPI f32 Lerp(f32 a, f32 b, f32 t);
TAPI f32 InvLerp(f32 a, f32 b, f32 v);
TAPI f32 Remap(f32 val, f32 iMin, f32 iMax, f32 oMin, f32 oMax);

TAPI glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, f32 t);

TAPI glm::vec2 RandomPointInCircle(f32 radius);
TAPI glm::vec3 RandomPointInSphere(f32 radius);

template<typename T> inline T Max(T x, T y) { return x > y ? x : y; }
template<typename T> inline T Min(T x, T y) { return x < y ? x : y; }
template <typename T> inline T Clamp(T& value, const T& low, const T& high) {
    return value < low ? low : (value > high ? high : value); 
}

template <typename T> int signof(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T> bool isInRange(T x, T min, T max) {
    return min <= x && x <= max;
}

TAPI uint32_t hash(const char* message, size_t message_length);

TAPI u32 countLeadingZeroes(u32 n);

TAPI glm::mat4 Position3DToModelMat(const glm::vec3& position, const glm::vec3& scale = glm::vec3(1), f32 rotation = 0.0, const glm::vec3& rotationAxis = {1,0,0});
TAPI glm::mat4 Position2DToModelMat(const glm::vec2& position, const glm::vec2& scale = glm::vec3(1), f32 rotation = 0.0, const glm::vec3& rotationAxis = {0,0,1});


}



#endif