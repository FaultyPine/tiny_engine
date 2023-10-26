#ifndef TINY_FS_H
#define TINY_FS_H

//#include "pch.h"
#include <string>

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

bool ReadFileContentsBinary(const char* filepath, void* backingBuffer, size_t size);
size_t GetFileSize(const char* filepath);
bool ReadEntireFile(const char* filename, std::string& str);

/// appends resource path to provided path
std::string ResPath(const std::string& path = "");

#endif