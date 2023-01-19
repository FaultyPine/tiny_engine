#include "math.h"

#include "tiny_engine/tiny_engine.h"

namespace Math {

 
bool isOverlappingRectSize2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2) {
    glm::vec2 r1 = pos1 + size1;
    glm::vec2 r2 = pos2 + size2;
    return isOverlappingRect2D(pos1, pos2, r1, r2);
}
// exclusionMax, exclusionMin, glm::vec2(randomPointBetweenVerts.x, randomPointBetweenVerts.z), glm::vec2(2)
bool isOverlappingRect2D(const glm::vec2& startPos1, const glm::vec2& endPos1, const glm::vec2& startPos2, const glm::vec2& endPos2) {
    if (startPos1.x > endPos2.x || startPos2.x > endPos1.x)
        return false;
    if (startPos1.y > endPos2.y || startPos2.y > endPos1.y)
        return false;
    return true;
}
bool isPointInRectangle(const glm::vec2& point, const glm::vec2& rectStart, const glm::vec2& rectEnd) {
    if (point.x > rectStart.x && point.x < rectEnd.x && point.y > rectStart.y && point.y < rectEnd.y)
        return true;
    return false;
}

bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist) {
    return glm::distance(pos1, pos2) <= dist;
}

glm::vec2 RandomPointInCircle(f32 radius) {
    glm::vec2 ret = glm::vec2(0);
    f32 len = sqrt(GetRandomf(0, 1)) * radius;
    f32 degrees = 2* PI_F * GetRandomf(0,1);
    ret.x = len * cos(degrees);
    ret.y = len * sin(degrees);
    return ret;
}
glm::vec3 RandomPointInSphere(f32 radius) {
    f32 u = GetRandomf(0,1);
    f32 x1 = GetRandomf(0,1);
    f32 x2 = GetRandomf(0,1);
    f32 x3 = GetRandomf(0,1);

    f32 mag = sqrt(x1*x1 + x2*x2 + x3*x3);
    x1 /= mag; x2 /= mag; x3 /= mag;

    // Math.cbrt is cube root
    f32 c = std::cbrt(u);

    return glm::vec3(x1*c, x2*c, x3*c);
}

f32 Lerp(f32 a, f32 b, f32 t) {
    return a * (1.0 - t) + (b * t);
}
f32 InvLerp(f32 a, f32 b, f32 v) {
    return (v - a) / (b - a);
}
f32 Remap(f32 v, f32 iMin, f32 iMax, f32 oMin, f32 oMax) {
    f32 t = InvLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}


}