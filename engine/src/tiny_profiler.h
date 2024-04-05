#pragma once
 
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include "tiny_defines.h"

 
#define TRACY_ENABLE
#define TRACY_EXPORTS
#include "external/tracy/tracy/Tracy.hpp"

#include "render/tiny_ogl.h"
#include "external/tracy/tracy/TracyOpenGL.hpp"

#define PROFILING 1

#if PROFILING
    #define PROFILE_SCOPE(name) ZoneScopedN(name);
    #define PROFILER_FRAME_MARK() FrameMark
    #define PROFILER_GPU_CONTEXT() TracyGpuContext
    #define PROFILER_GPU_SCOPE(name) TracyGpuZone(name)
    // should be used after the swap buffers func call
    #define PROFILER_GPU_FLUSH() TracyGpuCollect
#else
    #define PROFILE_SCOPE(name)
    #define PROFILER_FRAME_MARK()
    #define PROFILER_GPU_CONTEXT(name, nameSize)
    #define PROFILER_GPU_SCOPE(name)
    #define PROFILER_GPU_FLUSH() 
#endif
#define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCTION__)
