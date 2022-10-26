#include "math.h"

namespace Math {

 
bool isOverlappingRect2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2) {
    // get top-left corner and bottom-right corner of entities
    glm::vec2 l1 = pos1;
    glm::vec2 l2 = pos2;
    glm::vec2 r1 = l1 + size1;
    glm::vec2 r2 = l2 + size2;

    if (l1.x > r2.x || l2.x > r1.x)
        return false;

    //  this one
    if (l1.y > r2.y || l2.y > r1.y)
        return false;
    return true;
}

bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist) {
    return glm::distance(pos1, pos2) <= dist;
}


f32 Lerp(f32 a, f32 b, f32 t) {
    return (1.0f - t) * a + b * t;
}
f32 InvLerp(f32 a, f32 b, f32 v) {
    return (v - a) / (b - a);
}
f32 Remap(f32 iMin, f32 iMax, f32 oMin, f32 oMax, f32 v) {
    f32 t = InvLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}


}