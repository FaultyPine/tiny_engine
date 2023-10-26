#ifndef TINY_ASSETS_H
#define TINY_ASSETS_H

//#include "pch.h"
#include "tiny_defines.h"

/*

Ideas/Notes:

IDs can be integers (hashed strings, maybe the id is the hashed filepath of the asset?)
Assets will be different types... store pointers? Arrays of different types of assets? (Shader[], Texture[], etc)


hashmap with ID -> asset
can access with compile-time hashed strings (converted to ID) or with the raw id itself

Need to be able to:
Add assets (load from disk (async?))
"Remove" assets (unload from memory)
Access assets (asset may not exist... return optional<asset_type_or_whatever>)


Maybe use bump allocator for loading these to ease fragmentation. These will likely stick around for the lifetime
of the program.

*/

namespace Assets
{
    struct AssetID
    {
        u32 id = 0;
        operator bool() { return id != 0; }
    };
    u32 Load(const char* filepath);
    void Unload(u32 id);
    void* Get(u32 id);
}



#endif