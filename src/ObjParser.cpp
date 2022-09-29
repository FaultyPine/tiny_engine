#include "ObjParser.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_engine/tiny_obj_loader.h"

#include "tiny_engine/texture.h"
#include "tiny_engine/mesh.h" // for Vertex

using glm::vec3;
using glm::vec2;


void load_materials(const std::vector<tinyobj::material_t> & objmaterials, const std::string & tex_dir, std::vector<Material> & materials) {
    //extract the relevant material properties from the material_t format used by tinyobjloader
    //if a texture is defined, use the constructor that loads a texture
    for(auto mat : objmaterials) {
        vec3 diffuse_colour(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        if (mat.diffuse_texname.empty()) {
            materials.push_back(Material(diffuse_colour));
        }
        else {
            materials.push_back(Material(diffuse_colour, tex_dir + mat.diffuse_texname));
        }
    }
}


void load_obj(
    const char* filename, const char* matsDirectory, 
    std::vector<Vertex>& vertices, std::vector<u32>& indices,
    std::vector<Material>& materials) {
    //load a Wavefront .obj file at 'file' and store vertex coordinates as vec3 and face_inds as uvec3 of indices
    
    assert(vertices.size() == 0);
    assert(indices.size() == 0);
    assert(materials.size() == 0);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objmaterials;
    std::string err; std::string warn;

    //load all data in Obj file & 'triangulate'
    bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, 
        &warn, &err,
        filename, //model to load
        matsDirectory, //directory to search for materials
        true, //enable triangulation (mesh_t::num_face_vertices will always be 3 with this on)
        false // should fallback to white vertex colors if none are given
    ); 
    // NOTE: keep in mind triangulation will reorder indices and such so any lists here
    // may look different from the data in the file

    //boilerplate error handling
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!success) {
        std::cerr << err << " " << warn << std::endl;
    }

    // input data from tiny_obj_loader
    const std::vector<tinyobj::real_t>& attribVerts = attrib.vertices; // 3 floats per vertex
    const std::vector<tinyobj::real_t>& attribNorms = attrib.normals; // 3 floats per vertex
    const std::vector<tinyobj::real_t>& attribTexCoords = attrib.texcoords; // 2 floats per vertex
    const std::vector<tinyobj::real_t>& attribColors = attrib.colors; // 3 floats per vertex (optional)

    // go through all indices we got from tinyobjloader
    // and push them into a Vertex list so we can give it to opengl
    // in a proper VBO index buffer
    for (const auto& shape : shapes) {
        for (u32 i = 0; i < shape.mesh.indices.size(); i++) {
            Vertex v = {};
            // indexes into input data from obj file
            const tinyobj::index_t& tinyobjIdx = shape.mesh.indices.at(i);
            s32 vertexIndexBase = tinyobjIdx.vertex_index;
            vec3 pos = vec3(
                attribVerts.at( 3*vertexIndexBase+0 ),
                attribVerts.at( 3*vertexIndexBase+1 ),
                attribVerts.at( 3*vertexIndexBase+2 )
            );
            v.position = pos;
            if (!attribNorms.empty()) {
                s32 normalIndexBase = tinyobjIdx.normal_index;
                vec3 norm = vec3(
                    attribNorms.at( 3*normalIndexBase+0 ),
                    attribNorms.at( 3*normalIndexBase+1 ),
                    attribNorms.at( 3*normalIndexBase+2 )
                );
                v.normal = norm;
            }
            if (!attribTexCoords.empty()) {
                s32 texcoordIndexBase = tinyobjIdx.texcoord_index;
                vec2 texcoord = vec2(
                    attribTexCoords.at( 2*texcoordIndexBase+0 ),
                    attribTexCoords.at( 2*texcoordIndexBase+1 )
                );
                v.texCoords = texcoord;
            }
            if (!attribColors.empty()) {
                s32 colorIndexBase = tinyobjIdx.vertex_index; // using vert index for vert colors
                vec3 col = vec3(
                    attribColors.at( 3*colorIndexBase+0 ),
                    attribColors.at( 3*colorIndexBase+1 ),
                    attribColors.at( 3*colorIndexBase+2 )
                );
                v.color = col;
            }

            vertices.push_back(v);
            indices.push_back(indices.size());
        }
    }
    
    //conver materials and load textures
    load_materials(objmaterials, matsDirectory, materials);

    std::cout << "Loaded model " << filename << "." << std::endl;
}










