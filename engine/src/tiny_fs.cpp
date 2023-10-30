//#include "pch.h"
#include "tiny_fs.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "tiny_log.h"

static const char* glob_resourceDirectory = "";

void InitializeTinyFilesystem(const char* resourceDirectory)
{
    glob_resourceDirectory = resourceDirectory;
}

bool ReadFileContentsBinary(const char* filepath, void* backingBuffer, size_t size)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.read((char*)backingBuffer, size))
    {
        LOG_ERROR("Failed to read file %s", filepath);
        return false;
    }
    return true;
}

size_t GetFileSize(const char* filepath)
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    return size;
}

bool ReadEntireFile(const char* filename, std::string& str) {
    std::ifstream f(filename);
    if (f) {
        std::ostringstream ss;
        ss << f.rdbuf();
        str = ss.str();
        return true;
    }
    return false;
}
/// appends resource path to provided path
std::string ResPath(const std::string& path) {
    return glob_resourceDirectory + path;
}

