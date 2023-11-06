//#include "pch.h"
#include "ObjParser.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "external/obj_loader/tiny_obj_loader.h"

#include "render/texture.h"
#include "render/mesh.h" // for Vertex
#include "math/tiny_math.h"
#include "tiny_log.h"

// for std::filesystem::path
// TODO: replace this with custom string stuff
#include <filesystem>

using glm::vec3;
using glm::vec2;
using glm::vec4;

// if texture is defined/used, load the texture. Otherwise use material color
Material MaterialConvert(const tinyobj::material_t& mat, const std::string& texdir) {
    MaterialProp diffuse;
    MaterialProp ambient;
    MaterialProp specular;
    MaterialProp normal;
    MaterialProp* props[] = {&diffuse, &ambient, &specular, &normal};
    const f32* materialTypes[] = {mat.diffuse, mat.ambient, mat.specular, nullptr};
    const std::string* texnames[] = {&mat.diffuse_texname, &mat.ambient_texname, &mat.specular_texname, &mat.normal_texname};
    TextureMaterialType types[] = {TextureMaterialType::DIFFUSE, TextureMaterialType::AMBIENT, TextureMaterialType::SPECULAR, TextureMaterialType::NORMAL};
    TINY_ASSERT(ARRAY_SIZE(props) == ARRAY_SIZE(materialTypes) && ARRAY_SIZE(materialTypes) == ARRAY_SIZE(texnames));
    for (u32 i = 0; i < ARRAY_SIZE(props); i++) {
        MaterialProp& prop = *props[i];
        const f32* color = materialTypes[i];
        if (color) {
            vec3 colorvec = {color[0], color[1], color[2]};
            prop.color = vec4(colorvec, mat.dissolve);
        }
        if (!texnames[i]->empty()) {
            // Assume that textures are stored in "texdir"
            std::filesystem::path filename = std::filesystem::path(*texnames[i]);
            std::string path = std::filesystem::path(texdir).append(filename.string()).string();
            prop.texture = LoadTexture(path, TextureProperties::None(), true); // flip image upside down
            prop.hasTexture = true;
        }
    }
    // .obj Ns is [0, 1000]. Remap that to a resonable range
    f32 shininess = Math::Remap(mat.shininess, 0.0, 1000.0, 0.0, 50.0);
    Material meshMat = Material(diffuse, ambient, specular, normal, shininess, mat.name);
    return meshMat;
}
Vertex GetVertexFromTinyIdx(const tinyobj::index_t& tinyobjIdx, const tinyobj::attrib_t& attrib) {
    // input data from tiny_obj_loader
    const std::vector<tinyobj::real_t>& attribVerts = attrib.vertices; // 3 floats per vertex
    const std::vector<tinyobj::real_t>& attribNorms = attrib.normals; // 3 floats per vertex
    const std::vector<tinyobj::real_t>& attribTexCoords = attrib.texcoords; // 2 floats per vertex
    const std::vector<tinyobj::real_t>& attribColors = attrib.colors; // 3 floats per vertex (optional)
    Vertex v = {};
    // indexes into input data from obj file
    //const tinyobj::index_t& tinyobjIdx = shape.mesh.indices.at(i);
    s32 vertexIndexBase = tinyobjIdx.vertex_index;
    vec3 pos = vec3(
        attribVerts.at( 3*vertexIndexBase+0 ),
        attribVerts.at( 3*vertexIndexBase+1 ),
        attribVerts.at( 3*vertexIndexBase+2 )
    );
    v.position = pos;
    if (!attribNorms.empty()) {
        s32 normalIndexBase = tinyobjIdx.normal_index;
        if (normalIndexBase != -1) {
            vec3 norm = vec3(
                attribNorms.at( 3*normalIndexBase+0 ),
                attribNorms.at( 3*normalIndexBase+1 ),
                attribNorms.at( 3*normalIndexBase+2 )
            );
            v.normal = norm;
        }
    }
    if (!attribTexCoords.empty()) {
        s32 texcoordIndexBase = tinyobjIdx.texcoord_index;
        // obj files may have faces like   f 1530//780
        // this happens when the obj doesn't have texcoords
        if (texcoordIndexBase != -1) {
            vec2 texcoord = vec2(
                attribTexCoords.at( 2*texcoordIndexBase+0 ),
                attribTexCoords.at( 2*texcoordIndexBase+1 )
            );
            v.texCoords = texcoord;
        }
    }
    if (!attribColors.empty()) {
        s32 colorIndexBase = tinyobjIdx.vertex_index; // using vert index for vert colors
        if (colorIndexBase != -1) {
            vec3 col = vec3(
                attribColors.at( 3*colorIndexBase+0 ),
                attribColors.at( 3*colorIndexBase+1 ),
                attribColors.at( 3*colorIndexBase+2 )
            );
        v.color = col;
        }
    }
    return v;
}
Mesh MeshConvert(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attrib, const std::vector<Material>& allMaterials) {
    std::vector<Material> materials = {};
    std::vector<s32> usedMaterialIds = {};
    std::vector<Vertex> vertices = {};
    std::vector<u32> indices = {};
    for (u32 i = 0; i < shape.mesh.indices.size(); i++) {
        const tinyobj::index_t& tinyObjIdx = shape.mesh.indices.at(i);
        Vertex v = GetVertexFromTinyIdx(tinyObjIdx, attrib);
        
        // NOTE: shape.mesh.indices.size == (shape.mesh.material_ids.size * 3)
        s32 materialId = shape.mesh.material_ids.at(i / 3);
        if (materialId != -1) {
            s32 vertexMaterialId = 0;
            // don't push duplicate materials
            if (!std::count(usedMaterialIds.begin(), usedMaterialIds.end(), materialId)) {
                vertexMaterialId = materials.size();
                materials.push_back(allMaterials.at(materialId));
                usedMaterialIds.push_back(materialId);
            }
            v.materialId = vertexMaterialId;
        }

        vertices.push_back(v);
        indices.push_back(indices.size());
    }
    for (u32 i = 0; i < shape.lines.indices.size(); i++) {
        const tinyobj::index_t& tinyObjIdx = shape.lines.indices.at(i);
        Vertex v = GetVertexFromTinyIdx(tinyObjIdx, attrib);
        vertices.push_back(v);
        indices.push_back(indices.size());
    }

    return Mesh(vertices, indices, materials, shape.name);
}

void LoadMaterials(const std::vector<tinyobj::material_t>& objmaterials, const std::string& texdir, std::vector<Material>& materials) {
    //extract the relevant material properties from the material_t format used by tinyobjloader
    for (const tinyobj::material_t& mat : objmaterials) {
        Material convertedMat = MaterialConvert(mat, texdir);
        materials.push_back(convertedMat);
    }
}

std::vector<Mesh> LoadObjMesh(const char* filename, const char* matsDirectory) {
    //load a Wavefront .obj file at 'file' as a list of meshes
    std::vector<Mesh> ret = {};

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
        true // should fallback to white vertex colors if none are given
    ); 
    // NOTE: keep in mind triangulation will reorder indices and such so any lists here
    // may look different from the data in the file

    //boilerplate error handling
    if (!err.empty()) {
        LOG_ERROR("Load OBJ mesh error: %i", err);
    }
    if (!success) {
        LOG_WARN("Load OBJ mesh warning: %i", warn);
    }

    std::vector<Material> materials = {};
    //convert materials and load textures
    LoadMaterials(objmaterials, matsDirectory, materials);

    // a single obj might have multiple sub-meshes
    // iterate through each of those and load their verts/norms/materials/etc
    for (const tinyobj::shape_t& shape : shapes) {
        ret.emplace_back(MeshConvert(shape, attrib, materials));
    }

    LOG_INFO("Loaded model %s [mats: %i, submeshes: %i, verts: %i]", 
            filename, materials.size(), shapes.size(), attrib.vertices.size()/3);
    return ret;
}










