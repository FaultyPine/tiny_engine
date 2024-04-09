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
    #define PROFILE_GPU_SCOPE(name) TracyGpuZone(name)
    #define PROFILE_SCOPE(name) ZoneScopedN(name)
    #define PROFILER_FRAME_MARK() FrameMark
    #define PROFILER_GPU_CONTEXT() TracyGpuContext
    // should be used after the swap buffers func call
    #define PROFILER_GPU_FLUSH() TracyGpuCollect
#else
    #define PROFILE_SCOPE(name)
    #define PROFILER_FRAME_MARK()
    #define PROFILER_GPU_CONTEXT(name, nameSize)
    #define PROFILE_GPU_SCOPE(name)
    #define PROFILER_GPU_FLUSH() 
#endif
#define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCTION__)
#define PROFILE_FUNCTION_GPU() PROFILE_GPU_SCOPE(__FUNCTION__)
