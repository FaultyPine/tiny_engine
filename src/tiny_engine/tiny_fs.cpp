#include "pch.h"
#include "tiny_fs.h"


/// appends resource path to provided path
std::string ResPath(const std::string& path) {
    return "res/" + path;
}