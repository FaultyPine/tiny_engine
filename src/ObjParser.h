#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include "tiny_engine/pch.h" // for types


struct Vertex; struct Material;
void load_obj(
    const char* filename, const char* matsDirectory, 
    std::vector<Vertex>& vertices, std::vector<u32>& indices,
    std::vector<Material>& materials);

#endif