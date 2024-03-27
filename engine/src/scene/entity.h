#ifndef TINY_ENTITY_H
#define TINY_ENTITY_H

#include "tiny_defines.h"
#include "render/model.h"
#include "tiny_types.h"

// "entities" are just renderable positions with a bounding box right now
// made this mostly so I could have some engine-side notion of entities for experiments
// with algorithms like culling, spatial partitioning, etc

enum EntityFlags
{
    IS_DISABLED = 1 << 0,

    NUM_ENTITY_FLAGS,
};
STATIC_ASSERT(NUM_ENTITY_FLAGS < 32);

typedef u32 EntityRef;
#define ENTITY_NAME_MAX_LENGTH 50
struct Entity
{
    Transform transform = {};
    Model model = {};
    BoundingBox bounds = {};
    EntityRef id = U32_INVALID_ID;
    u32 flags = 0;
    s8 name[ENTITY_NAME_MAX_LENGTH];
    
    operator bool() { return isValid(); }
    Entity() = default;
    inline bool isValid() { return id != U32_INVALID_ID; }
};

struct Arena;
void InitializeEntitySystem(Arena* arena);

TAPI EntityRef CreateEntity(const char* name, const Model& model, const Transform& tf, u32 flags = 0);
TAPI bool DestroyEntity(EntityRef ent);
TAPI Entity& GetEntity(EntityRef ent);
TAPI Entity& GetEntity(const char* name);
// if dst is nullptr, this returns the *number* of renderable entities
// if dst is not nullptr, we fill in the buffer
TAPI void GetRenderableEntities(EntityRef* dst, u32* numEntities);

#endif