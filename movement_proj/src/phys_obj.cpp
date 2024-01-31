#include "phys_obj.h"


void physobj_update_position(PhysicsObject& obj, float dt)
{
    glm::vec2 velocity = obj.position - obj.prev_position;
    obj.prev_position = obj.position;
    // verlet integration
    obj.position = obj.position + velocity + obj.acceleration * dt * dt;
    // reset accel every update
    obj.acceleration = glm::vec2(0);
}

void physobj_accelerate(PhysicsObject& obj, glm::vec2 accel)
{
    obj.acceleration += accel;
}
