#ifndef TINY_PHYSICS_H
#define TINY_PHYSICS_H


#include "tiny_defines.h"

struct btDiscreteDynamicsWorld;
struct Arena;
struct Model;
struct Transform;

struct PhysicsWorld
{
    btDiscreteDynamicsWorld* dynamicsWorld = 0;
};

void InitializePhysics(Arena* arena);
void PhysicsTick();
TAPI void PhysicsDebugRender();
TAPI void PhysicsAddModel(const Model& model, const Transform& tf);

#endif