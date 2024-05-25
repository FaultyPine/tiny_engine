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
    DISABLED = 1,

    NUM_ENTITY_FLAGS,
};
STATIC_ASSERT(NUM_ENTITY_FLAGS < 32);

typedef u32 EntityRef;
#define ENTITY_NAME_MAX_LENGTH 50
struct EntityData
{
    Transform transform = {};
    Model model = {}; // TODO: "renderable" shouldn't hardcoded to Model... Sprites too? this is getting into ECS territory
    BoundingBox bounds = {};
    EntityRef id = U32_INVALID_ID;
    u32 flags = 0;
    s8 name[ENTITY_NAME_MAX_LENGTH];
    
    operator bool() { return isValid(); }
    EntityData() = default;
    inline bool isValid() { return id != U32_INVALID_ID; }
};

typedef std::unordered_map<u32, EntityData> EntityMap;

struct EntityRegistry
{
    EntityMap entMap = {};
    u32 entityCreationIndex = 0;
};

struct Arena;
namespace Entity
{

void InitializeEntitySystem(Arena* arena);

TAPI EntityRef CreateEntity(
    const char* name, 
    const Transform& tf = {}, 
    u32 flags = 0);
TAPI bool DestroyEntity(EntityRef ent);
TAPI EntityData& GetEntity(EntityRef ent);
TAPI EntityData& GetEntity(const char* name);

TAPI void SetFlag(EntityRef ent, EntityFlags flag, bool enabled);
TAPI void SetFlag(EntityData& ent, EntityFlags flag, bool enabled);
TAPI bool IsFlag(EntityRef ent, EntityFlags flag);
TAPI bool IsFlag(const EntityData& ent, EntityFlags flag);
TAPI void SetTransform(EntityRef ent, const Transform& tf);
TAPI bool HasRenderable(EntityRef ent);
TAPI bool AddRenderable(EntityRef ent, const Model& model);
TAPI void OverwriteRenderable(EntityRef ent, const Model& model);
// if dst is nullptr, this returns the *number* of renderable entities
// if dst is not nullptr, we fill in the buffer
TAPI void GetRenderableEntities(EntityRef* dst, u32* numEntities);

} // namespace Entity


#endif