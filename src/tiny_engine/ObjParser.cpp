#include "ObjParser.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_engine/tiny_obj_loader.h"

#include "tiny_engine/texture.h"
#include "tiny_engine/mesh.h" // for Vertex

using glm::vec3;
using glm::vec2;
using glm::vec4;


void load_materials(const std::vector<tinyobj::material_t>& objmaterials, const std::string& tex_dir, std::vector<Material>& materials) {
    //extract the relevant material properties from the material_t format used by tinyobjloader
    //if a texture is defined, use the constructor that loads a texture
    for (tinyobj::material_t mat : objmaterials) {
        MaterialProp diffuse;
        MaterialProp ambient;
        MaterialProp specular;
        MaterialProp* props[] = {&diffuse, &ambient, &specular};
        f32* materialTypes[] = {mat.diffuse, mat.ambient, mat.specular};
        std::string* texnames[] = {&mat.diffuse_texname, &mat.ambient_texname, &mat.specular_texname};
        TextureMaterialType types[] = {TextureMaterialType::DIFFUSE, TextureMaterialType::AMBIENT, TextureMaterialType::SPECULAR};
        assert(ARRAY_SIZE(props) == ARRAY_SIZE(materialTypes) && ARRAY_SIZE(materialTypes) == ARRAY_SIZE(texnames));

        for (u32 i = 0; i < ARRAY_SIZE(props); i++) {
            MaterialProp& prop = *props[i];
            f32* color = materialTypes[i];
            vec3 colorvec = {color[0], color[1], color[2]};
            prop.color = vec4(colorvec, mat.dissolve);
            if (!texnames[i]->empty()) {
                prop.texture = LoadTexture(tex_dir + *texnames[i]);
                prop.texture.type = types[i];
                prop.hasTexture = true;
            }
        }
        Material meshMat = Material(diffuse, ambient, specular, mat.name);
        meshMat.shininess = mat.shininess;
        materials.push_back(meshMat);
    }
}

std::vector<Mesh> load_obj(
    const Shader& shader, const char* filename, const char* matsDirectory) {
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
    
    std::vector<Material> materials = {};
    //convert materials and load textures
    load_materials(objmaterials, matsDirectory, materials);


    // a single obj might have multiple sub-meshes
    // iterate through each of those and load their verts/norms/materials/etc
    for (const tinyobj::shape_t& shape : shapes) {
        // assert all material id's are the same... not currently supporting different materials on a single mesh
        assert("Multiple materials not supported for single mesh" && std::equal(shape.mesh.material_ids.begin() + 1, shape.mesh.material_ids.end(), shape.mesh.material_ids.begin()));

        std::vector<Vertex> vertices = {};
        std::vector<u32> indices = {};
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
                // obj files may have faces like   f 1530//780
                // this happens when the obj doesn't have texcoords
                if (texcoordIndexBase != -1) {
                    vec2 texcoord = vec2(
                        attribTexCoords.at( 2*texcoordIndexBase+0 ),
                        attribTexCoords.at( 2*texcoordIndexBase+1 )
                    );
                    v.texCoords = texcoord;
                }
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
        s32 materialId = shape.mesh.material_ids[0];
        Material material = {};
        if (materialId != -1)
            material = materials.at(materialId);
        //std::cout << shape.name << " material id " << materialId << " " << materials.at(materialId).name << "\n";
        //std::cout << VecToStr(materials.at(materialId).diffuseMat.color) << "\n";
        
        ret.emplace_back(Mesh(shader, vertices, indices, 
            {}, material));
    }

    std::cout << "Loaded model " << filename << "." << std::endl;
    return ret;
}










