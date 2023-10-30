#ifndef TINY_FS_H
#define TINY_FS_H

//#include "pch.h"
#include "tiny_defines.h"
#include <string>

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

TAPI bool ReadFileContentsBinary(const char* filepath, void* backingBuffer, size_t size);
TAPI size_t GetFileSize(const char* filepath);
TAPI bool ReadEntireFile(const char* filename, std::string& str);

/// appends resource path to provided path
TAPI std::string ResPath(const std::string& path = "");

void InitializeTinyFilesystem(const char* resourceDirectory);

#endif