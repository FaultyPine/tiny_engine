#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

//#include "pch.h"
#include "tiny_defines.h"
#include "mesh.h"

TAPI std::vector<Mesh> LoadObjMesh(const char* filename, const char* matsDirectory);

#endif