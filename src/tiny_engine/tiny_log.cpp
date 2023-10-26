#include "tiny_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static u32 LOG_LEVELS_ENABLED = 0;

// defaults
#define LOG_LEVEL_FATAL_ENABLED 1
#define LOG_LEVEL_ERROR_ENABLED 1
#define LOG_LEVEL_WARN_ENABLED 1
#define LOG_LEVEL_INFO_ENABLED 1
#define LOG_LEVEL_DEBUG_ENABLED 1
#define LOG_LEVEL_TRACE_ENABLED 1

bool InitializeLogger()
{
    LOG_LEVELS_ENABLED |= LOG_LEVEL_FATAL_ENABLED;
    LOG_LEVELS_ENABLED |= LOG_LEVEL_ERROR_ENABLED;
    LOG_LEVELS_ENABLED |= LOG_LEVEL_WARN_ENABLED;
    LOG_LEVELS_ENABLED |= LOG_LEVEL_INFO_ENABLED;
    LOG_LEVELS_ENABLED |= LOG_LEVEL_DEBUG_ENABLED;
    LOG_LEVELS_ENABLED |= LOG_LEVEL_TRACE_ENABLED;
    return true;
}
void ShutdownLogger()
{
    // nothing going on here for now. When logging to file exists this will do something
}

void SetLogLevel(LogLevel level, bool toggle)
{
    SET_NTH_BIT(LOG_LEVELS_ENABLED, level, toggle);
}

void LogMessage(LogLevel level, const char* message, ...)
{
    if (LOG_LEVELS_ENABLED & level == 0)
    {
        return;
    }
    static const char* level_strings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};

    constexpr u32 log_message_limit = 16000;
    char out_msg[log_message_limit]; // hardcoded log limit...

    va_list args;
    va_start(args, message);
    vsnprintf(out_msg, log_message_limit, message, args);
    va_end(args);

    // append log level to message
    printf("%s %s", level_strings[level], out_msg);
}


// yoinked from raylib
// https://github.com/raysan5/raylib/blob/master/src/rcore.c#L7169
const char *TextFormat(const char *text, ...)
{
#ifndef MAX_TEXTFORMAT_BUFFERS
    #define MAX_TEXTFORMAT_BUFFERS      12        // Maximum number of static buffers for text formatting
#endif
#ifndef MAX_TEXT_BUFFER_LENGTH
    #define MAX_TEXT_BUFFER_LENGTH   1024        // Maximum size of static text buffer
#endif

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}