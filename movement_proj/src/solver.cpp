#include "solver.h"
#include "tiny_log.h"

namespace Solver
{

void update(PhysicsObject* objs, u32 numObjs, float dt)
{
    Solver::apply_gravity(objs, numObjs);
    Solver::apply_constraints(objs, numObjs);
    //Solver::solve_collisions(objs, numObjs);

    Solver::update_positions(objs, numObjs, dt);
}

void update_positions(PhysicsObject* objs, u32 numObjs,float dt)
{
    for (u32 i = 0; i < numObjs; i++)
    {
        physobj_update_position(objs[i], dt);
    }
}

void apply_gravity(PhysicsObject* objs, u32 numObjs)
{
    constexpr f32 gravity_coeff = 100.0f;
    for (u32 i = 0; i < numObjs; i++)
    {
        physobj_accelerate(objs[i], glm::vec2(0.0f, gravity_coeff));
    }
}

void apply_circle_constraint(PhysicsObject* objs, u32 numObjs, glm::vec2 position, f32 radius)
{
    for (u32 i = 0; i < numObjs; i++)
    {
        PhysicsObject& obj = objs[i];
        glm::vec2 to_obj = obj.position - position; // obj -> circle constraint center
        f32 dist = glm::length(to_obj);
        if (dist > (radius - obj.radius))
        {
            glm::vec2 displacement = to_obj / dist;
            obj.position = position + displacement * (radius - obj.radius);
        }
    }
}

void apply_constraints(PhysicsObject* objs, u32 numObjs)
{
    apply_circle_constraint(objs, numObjs, glm::vec2(10.0f, 10.0f), 100.0f);
}

void solve_collisions(PhysicsObject* objs, u32 numObjs)
{
    // brute force
    for (u32 i = 0; i < numObjs; i++)
    {
        PhysicsObject& obj1 = objs[i];
        for (u32 k = i+1; k < numObjs; k++)
        {
            PhysicsObject& obj2 = objs[k];
            glm::vec2 collision_axis = obj1.position - obj2.position;
            f32 dist = glm::length(collision_axis);
            f32 min_dist = obj1.radius + obj2.radius;
            if (dist < min_dist)
            {
                glm::vec2 displacement = collision_axis / dist;
                f32 delta = min_dist - dist;
                obj1.position += 0.5f * delta * displacement;
                obj2.position -= 0.5f * delta * displacement;
            }
        }
    }
}

}