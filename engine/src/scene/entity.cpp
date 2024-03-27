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

EntityRef GenerateEntityID(const char* name = nullptr)
{
    EntityRef result = 0;
    EntityRegistry& registry = GetRegistry();
    if (!name)
    {
        EntityRegistry& registry = GetRegistry();
        result = registry.entityCreationIndex;
    }
    else
    {
        result = HashBytes((u8*)name, strnlen(name, ENTITY_NAME_MAX_LENGTH));
    }
    TINY_ASSERT(result != U32_INVALID_ID); // TODO: handle this gracefully
    return result;
}

void InitializeEntitySystem(Arena* arena)
{
    EntityRegistry* registryMem = arena_alloc_and_init<EntityRegistry>(arena);
    GetEngineCtx().entityRegistry = registryMem;
    // dummy entity with bad id so we can return it on failure from methods like GetEntity
    registryMem->entMap[U32_INVALID_ID] = {};
}

EntityRef CreateEntity(const char* name, const Model& model, const Transform& tf, u32 flags)
{
    EntityRegistry& registry = GetRegistry();
    Entity ent = {};
    ent.model = model;
    ent.transform = tf;
    ent.flags = flags;
    u32 entityID = 0;
    if (name)
    {
        // if this entity has a name, use the name's hash as the id
        // this is so we can lookup entities by name
        size_t strlength = strnlen(name, ENTITY_NAME_MAX_LENGTH);
        TMEMCPY(ent.name, name, strlength);
        entityID = HashBytes((u8*)name, strlength);
    }
    else
    {
        // if no name, the entity id is just a incrementally increasing num
        TMEMSET(ent.name, 0, ENTITY_NAME_MAX_LENGTH);
        entityID = registry.entityCreationIndex;
    }
    TINY_ASSERT(entityID != U32_INVALID_ID); // TODO: handle this gracefully
    ent.id = entityID;
    // we increment this every time, even if it's not what we use for the id.
    registry.entityCreationIndex++;
    registry.entMap[ent.id] = ent;
    return ent.id;
}

bool DestroyEntity(EntityRef ent)
{
    EntityRegistry& registry = GetRegistry();
    Entity& entity = GetEntity(ent);
    if (entity.id == U32_INVALID_ID)
    {
        return false;
    }
    entity.model.Delete();
    registry.entMap.erase(ent);
    return true;
}

Entity& GetEntity(EntityRef ref)
{
    EntityRegistry& registry = GetRegistry();
    if (registry.entMap.count(ref) > 0)
    {
        return registry.entMap[ref];
    }
    return registry.entMap[U32_INVALID_ID]; // if doesn't exist, return our dummy
}

Entity& GetEntity(const char* name)
{
    u32 namehash = HashBytes((u8*)name, strnlen(name, ENTITY_NAME_MAX_LENGTH));
    EntityRegistry& registry = GetRegistry();
    if (registry.entMap.count(namehash) > 0)
    {
        return registry.entMap[namehash];
    }
    return registry.entMap[U32_INVALID_ID]; // if doesn't exist, return our dummy

}

void GetRenderableEntities(EntityRef* dst, u32* numEntities)
{
    EntityRegistry& registry = GetRegistry();
    if (!dst)
    {
        *numEntities = registry.entMap.size();
        return;
    }
    u32 i = 0;
    for (const auto& [ref, ent] : registry.entMap)
    {
        if ((ent.flags & EntityFlags::IS_DISABLED) == 0)
        {
            dst[i] = ref;
        }
        else
        {
            dst[i] = U32_INVALID_ID;
        }
        i++;
    }
}