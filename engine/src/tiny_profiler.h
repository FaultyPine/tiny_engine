#pragma once
 

#include "tiny_defines.h"

 
#define TRACY_ENABLE
#ifdef TEXPORT
#define TRACY_EXPORTS
#else
#define TRACY_IMPORTS
#endif
#include "external/tracy/tracy/Tracy.hpp"

#include "render/tiny_ogl.h"
#include "external/tracy/tracy/TracyOpenGL.hpp"

#define PROFILING 1

#if PROFILING
#define MEM_PROFILING 0
    #define PROFILE_GPU_SCOPE(name) TracyGpuZone(name)
    #define PROFILE_SCOPE(name) ZoneScopedN(name)
    #define PROFILER_FRAME_MARK() FrameMark
    #define PROFILER_GPU_CONTEXT() TracyGpuContext
    // should be used after the swap buffers func call
    #define PROFILER_GPU_FLUSH() TracyGpuCollect
#if MEM_PROFILING
    #define PROFILE_ALLOC(ptr, size, name) TracyAllocN(ptr, size, name)
    #define PROFILE_FREE(ptr, name) TracyFreeN(ptr, name)
#else 
    #define PROFILE_ALLOC(ptr, size, name) 
    #define PROFILE_FREE(ptr, name) 
#endif

#else // #if PROFILING

    #define PROFILE_SCOPE(name)
    #define PROFILER_FRAME_MARK()
    #define PROFILER_GPU_CONTEXT(name, nameSize)
    #define PROFILE_GPU_SCOPE(name)
    #define PROFILER_GPU_FLUSH() 
    #define PROFILE_ALLOC(ptr, size, name)
    #define PROFILE_FREE(ptr, name)
#endif
#define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_FUNCTION_GPU() PROFILE_GPU_SCOPE(__FUNCTION__)
