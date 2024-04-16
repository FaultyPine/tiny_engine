#include "tiny_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>

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
    SetLogLevel(LOG_LEVEL_FATAL, LOG_LEVEL_FATAL_ENABLED);
    SetLogLevel(LOG_LEVEL_ERROR, LOG_LEVEL_ERROR_ENABLED);
    SetLogLevel(LOG_LEVEL_WARN, LOG_LEVEL_WARN_ENABLED);
    SetLogLevel(LOG_LEVEL_INFO, LOG_LEVEL_INFO_ENABLED);
    SetLogLevel(LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_ENABLED);
    SetLogLevel(LOG_LEVEL_TRACE, LOG_LEVEL_TRACE_ENABLED);
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

#define TERMINAL_COLORED_OUTPUT_ENABLED 1

static const char* level_strings[6] = {"[FATAL]", "[ERROR]", "[WARN]", "[INFO]", "[DEBUG]", "[TRACE]"};
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
static int terminal_colors[6] = {4, 4, 6, 2, 1, 1};
#else
static const char* terminal_colors[6] = {"\033[0;31m", "\033[0;31m", "\033[0;33m", "\033[0;32m", "\033[0;34m", "\033[0;34m"};
#endif

void SetTerminalColor(LogLevel level)
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (level < 0)
    {
        int defaultWhite = 7;
        SetConsoleTextAttribute(hConsole, defaultWhite);
    }
    else
    {
        SetConsoleTextAttribute(hConsole, terminal_colors[level]);
    }
#else
    if (level < 0)
    {
        printf("\033[0m");
    }
    else
    {
        printf("%s", terminal_colors[level]);
    }
#endif
}

void LogMessage(LogLevel level, const char* message, ...)
{
    if ((LOG_LEVELS_ENABLED & (1 << level)) == 0)
    {
        return;
    }

    constexpr u32 log_message_limit = 16000;
    char out_msg[log_message_limit]; // hardcoded log limit...

    va_list args;
    va_start(args, message);
    s32 bytesWritten = vsnprintf(out_msg, log_message_limit, message, args);
    va_end(args);
    TINY_ASSERT(bytesWritten < log_message_limit);

    // append (optional)color and log level to message
#if TERMINAL_COLORED_OUTPUT_ENABLED
    SetTerminalColor(level);
    printf("%s %s\n", level_strings[level], out_msg);
    SetTerminalColor((LogLevel)-1);
#else
    printf("%s %s\n", level_strings[level], out_msg);
#endif
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