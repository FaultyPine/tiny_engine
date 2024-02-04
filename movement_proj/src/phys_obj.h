#pragma once

#include "math/tiny_math.h"

struct PhysicsObject
{
    glm::vec2 position = glm::vec2(0);
    glm::vec2 prev_position = glm::vec2(0);
    glm::vec2 acceleration = glm::vec2(0);
    f32 radius = 10.0f;
};  

void physobj_update_position(PhysicsObject& ball, float dt);
void physobj_accelerate(PhysicsObject& ball, glm::vec2 accel);


