#ifndef TINY_MATH_H
#define TINY_MATH_H

#include "pch.h"

namespace Math {



bool isOverlappingRect2D(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2);
bool isPositionNear(const glm::vec2& pos1, const glm::vec2& pos2, f32 dist);





}



#endif