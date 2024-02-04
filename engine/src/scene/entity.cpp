#include "entity.h"

#include "mem/tiny_arena.h"
#include "tiny_engine.h"
#include "tiny_log.h"
#include "render/model.h"

#include <unordered_map>


typedef std::unordered_map<u32, Entity> EntityMap;

struct EntityRegistry
{
    EntityMap entMap = {};
    u32 entityCreationIndex = 0;
};

static EntityRegistry& GetRegistry()
{
    return *GetEngineCtx().entityRegistry;
}

void InitializeEntitySystem(Arena* arena)
{
    EntityRegistry* registryMem = (EntityRegistry*)arena_alloc(arena, sizeof(EntityRegistry));
    GetEngineCtx().entityRegistry = registryMem;
    TMEMSET(registryMem, 0, sizeof(EntityRegistry));
    new(&registryMem->entMap) EntityMap();
}

EntityRef CreateBlankEntity()
{
    EntityRegistry& registry = GetRegistry();
    Entity ent = {};
    ent.id = registry.entityCreationIndex++;
    registry.entMap[ent.id] = ent;
    return ent.id;
}

Entity& GetEntity(EntityRef ref)
{
    return GetRegistry().entMap[ref];
}