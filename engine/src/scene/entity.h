#ifndef TINY_ENTITY_H
#define TINY_ENTITY_H

#include "tiny_defines.h"
#include "render/model.h"
#include "tiny_types.h"

// this isn't meant to be a full ECS system
// engine "entities" are just renderable positions with a bounding box right now
// made this mostly so I could have some engine-side notion of entities for experiments
// with algorithms like culling, spatial partitioning, etc

enum EntityFlags
{
    IS_DISABLED = 1 << 0,

    NUM_ENTITY_FLAGS,
};
STATIC_ASSERT(NUM_ENTITY_FLAGS < 32);

typedef u32 EntityRef;

struct Entity
{
    Transform transform = {};
    Model model = {};
    BoundingBox bounds = {};
    EntityRef id = U32_INVALID_ID;
    u32 flags = 0;
};

struct Arena;
void InitializeEntitySystem(Arena* arena);

EntityRef CreateBlankEntity();
Entity& GetEntity(EntityRef ref);


#endif