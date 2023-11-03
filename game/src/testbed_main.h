#pragma once

//#include "pch.h"
#include "tiny_defines.h"

struct AppRunCallbacks;
TAPI void GetTestbedAppRunCallbacks(AppRunCallbacks* out);
TAPI void testbed_standalone_entrypoint(int argc, char *argv[]);