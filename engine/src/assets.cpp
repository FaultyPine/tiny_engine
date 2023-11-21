//#include "pch.h"
#include "assets.h"
#include "tiny_alloc.h"
#include "tiny_fs.h"
#include "tiny_log.h"

#include <unordered_map>

namespace Assets
{
#define PREALLOCATED_ASSET_MEM_MB 10

// TODO: put in engine mem
static std::unordered_map<u32, void*> assetRecord = {};

/*Arena& GetAssetArena()
{
    static Arena assetArena = {};
    if (assetArena.backing_mem == nullptr)
    {
        u32 arenaSizeBytes = MEGABYTES_BYTES(PREALLOCATED_ASSET_MEM_MB);
        void* backingArenaMem = TSYSALLOC(arenaSizeBytes);
        assetArena = arena_init(backingArenaMem, arenaSizeBytes);
    }
    return assetArena;
}*/

u32 Load(const char* filepath)
{
    size_t size = GetFileSize(filepath);

    //Arena& assetArena = GetAssetArena();
    //void* assetData = arena_alloc(&assetArena, size);
    void* assetData = TSYSALLOC(size);
    u32 assetID = 0;
    if (ReadFileContentsBinary(filepath, assetData, size))
    {
        assetID = std::hash<std::string>{}(filepath);
        LOG_INFO("Loaded asset ID = %i filepath = %s", assetID, filepath);
        assetRecord[assetID] = assetData;
    }
    else
    {
        TINY_ASSERT(false && "[ERROR] Failed to read file");
    }
    return assetID;
}
void Unload(u32 id)
{
    if (assetRecord.count(id))
    {
        void* assetData = assetRecord[id];
        assetRecord.erase(id);
        TSYSFREE(assetData);
    }
}
void* Get(u32 id)
{
    if (assetRecord.count(id))
    {
        return assetRecord[id];
    }
    return nullptr;
}


}