#include "pch.h"
#include "assets.h"
#include "tiny_alloc.h"


namespace Assets
{
#define PREALLOCATED_ASSET_MEM_MB 10
static std::unordered_map<u32, void*> assetRecord = {};

/*Arena& GetAssetArena()
{
    static Arena assetArena = {};
    if (assetArena.backing_mem == nullptr)
    {
        u32 arenaSizeBytes = MEGABYTES_BYTES(PREALLOCATED_ASSET_MEM_MB);
        void* backingArenaMem = TINY_ALLOC(arenaSizeBytes);
        assetArena = arena_init(backingArenaMem, arenaSizeBytes);
    }
    return assetArena;
}*/

u32 Load(const char* filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    //Arena& assetArena = GetAssetArena();
    //void* assetData = arena_alloc(&assetArena, size);
    void* assetData = TINY_ALLOC(size);
    u32 assetID = 0;
    if (file.read((char*)assetData, size))
    {
        assetID = std::hash<std::string>{}(filepath);
        std::cout << "Loaded asset ID = " << assetID << " filepath = " << filepath;
        assetRecord[assetID] = assetData;
    }
    else
    {
        std::cout << "[ERROR] Failed to read file " << filepath << "\n";
        ASSERT(false && "[ERROR] Failed to read file");
    }
    return assetID;
}
void Unload(u32 id)
{
    if (assetRecord.count(id))
    {
        void* assetData = assetRecord[id];
        assetRecord.erase(id);
        TINY_DELETE(assetData);
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