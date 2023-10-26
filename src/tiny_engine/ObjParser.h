#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

//#include "pch.h"
#include "tiny_engine/mesh.h"

std::vector<Mesh> LoadObjMesh(const char* filename, const char* matsDirectory);

#endif