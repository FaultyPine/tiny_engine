#ifndef TINY_LOG_H
#define TINY_LOG_H

#include "tiny_defines.h"

// NOTE: logger includes both logging *and* assertions

enum LogLevel
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,

    LOG_NUM_LEVELS,
};

#ifdef _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK __builtin_trap()
#endif

bool InitializeLogger();
void ShutdownLogger();
void SetLogLevel(LogLevel level, bool toggle);
const char* TextFormat(const char *text, ...);
TAPI void LogMessage(LogLevel level, const char* message, ...);

#define LOG_FATAL(message, ...) LogMessage(LOG_LEVEL_FATAL, message, __VA_ARGS__)
#define LOG_ERROR(message, ...) LogMessage(LOG_LEVEL_ERROR, message, __VA_ARGS__)
#define LOG_WARN(message, ...) LogMessage(LOG_LEVEL_WARN, message, __VA_ARGS__)
#define LOG_INFO(message, ...) LogMessage(LOG_LEVEL_INFO, message, __VA_ARGS__)
#define LOG_DEBUG(message, ...) LogMessage(LOG_LEVEL_DEBUG, message, __VA_ARGS__)
#define LOG_TRACE(message, ...) LogMessage(LOG_LEVEL_TRACE, message, __VA_ARGS__)


#ifdef TINY_ASSERTIONS_ENABLED
#include <intrin.h>
#define TINY_ASSERT(x) \
    if (!(x)) { LOG_FATAL("%s | %s:%i\n", #x, __FILE__, __LINE__); DEBUG_BREAK; *((char*)0)=0; }
#else
#define TINY_ASSERT(x)
#endif

#endif