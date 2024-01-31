#pragma once

#include "tiny_defines.h"
#include "phys_obj.h"

namespace Solver
{

void update_positions(PhysicsObject* objs, u32 numObjs, float dt);

void apply_gravity(PhysicsObject* objs, u32 numObjs);

void update(PhysicsObject* objs, u32 numObjs, float dt);

void apply_constraints(PhysicsObject* objs, u32 numObjs);

void solve_collisions(PhysicsObject* objs, u32 numObjs);

}