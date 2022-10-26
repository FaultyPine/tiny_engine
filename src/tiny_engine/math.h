#ifndef TINY_MATH_H
#define TINY_MATH_H

#include "pch.h"

namespace Math {



bool isOverlappingRect2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2);
bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist);

f32 Lerp(f32 a, f32 b, f32 t);
f32 InvLerp(f32 a, f32 b, f32 v);
f32 Remap(f32 iMin, f32 iMax, f32 oMin, f32 oMax, f32 v);


}



#endif