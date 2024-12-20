//#include "pch.h"
#include "tiny_math.h"

#include "tiny_engine.h"

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
    f32 x1 = GetRandomf(-1,1);
    f32 x2 = GetRandomf(-1,1);
    f32 x3 = GetRandomf(-1,1);

    f32 mag = sqrt(x1*x1 + x2*x2 + x3*x3);
    x1 /= mag; x2 /= mag; x3 /= mag;

    f32 c = std::cbrt(radius);

    return glm::vec3(x1*c, x2*c, x3*c);
}

f32 Lerp(f32 a, f32 b, f32 t) {
    return a * (1.0 - t) + (b * t);
}
glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, f32 t) {
    return a * (f32)(1.0 - t) + (b * t);
}
f32 InvLerp(f32 a, f32 b, f32 v) {
    return (v - a) / (b - a);
}
f32 Remap(f32 v, f32 iMin, f32 iMax, f32 oMin, f32 oMax) {
    f32 t = InvLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}

u32 countLeadingZeroes(u32 n) {
    return n == 0 ? 0 : log2(n & -n);
}

glm::mat4 Position3DToModelMat(const glm::vec3& position, const glm::vec3& scale, f32 rotation, const glm::vec3& rotationAxis) {
    glm::mat4 model = glm::mat4(1);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    model = glm::rotate(model, glm::radians(rotation), rotationAxis); 
    return model;
}

glm::mat4 Position2DToModelMat(const glm::vec2& position, const glm::vec2& scale, f32 rotation, const glm::vec3& rotationAxis) {
    // set up transform of the actual sprite
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));  

    // rotation is about the "middle" of the shape. 
    // if you want rotation about some other part of the shape, use the overloaded DrawShape and make
    // your own model matrix
    model = glm::translate(model, glm::vec3(0.5f * scale.x, 0.5f * scale.y, 0.0f)); 
    model = glm::rotate(model, glm::radians(rotation), rotationAxis); 
    model = glm::translate(model, glm::vec3(-0.5f * scale.x, -0.5f * scale.y, 0.0f));

    model = glm::scale(model, glm::vec3(scale.x, scale.y, 1.0f));  
    return model;
}

uint32_t hash(const char* message, size_t message_length)
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

}