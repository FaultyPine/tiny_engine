#pragma once
 
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include "tiny_defines.h"

// Author(s): The Cherno and davechurchill with some modifications by me
// https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e

// chrome tracing event format
// https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview#

// HOW TO USE:
// Enable profiling -> run game -> close game -> open google chrome -> enter chrome://tracing into
// URL -> drag profiling_results.json onto that window

#define TRACY_ENABLE
#define TRACY_EXPORTS
#include "external/tracy/tracy/Tracy.hpp"

#define PROFILING 1

#if PROFILING
    #define PROFILE_SCOPE(name) ZoneScopedN(name);
    #define PROFILER_FRAME_END() FrameMark
#else
    #define PROFILE_SCOPE(name)
    #define PROFILER_FRAME_END()
#endif
#define PROFILE_FUNCTION()  PROFILE_SCOPE(__FUNCTION__)

void ProfilerBegin();
void ProfilerEnd();