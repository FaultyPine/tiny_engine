#include "tiny_fs.h"


/// appends resource path to provided path
std::string UseResPath(const std::string& path) {
    return "res/" + path;
}