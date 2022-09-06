#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

// TODO: put headers here so this can be standalone

// ex of using tiny_obj_loader
// https://github.com/canmom/rasteriser/blob/master/fileloader.cpp

// https://stackoverflow.com/questions/67638439/how-do-i-draw-an-obj-file-in-opengl-using-tinyobjloader

// setting up proper indices list
// https://github.com/huamulan/OpenGL-tutorial/blob/master/common/vboindexer.cpp

using glm::vec3;
using glm::vec2;

// synonymous with canmom's rasteriser
struct Triangle {
    vec3 vertices;
    vec3 normals;
    vec3 uvs;
    u32 material;

    Triangle(vec3 verts, vec3 norms, vec3 uvs, u32 mat) {
        vertices = verts; normals = norms, this->uvs = uvs; material = mat;
    }
};
// synonymous with canmom's rasteriser except with my own Texture struct
struct Material {
    vec3 diffuseColor;
    bool hasTexture;
    std::string diffuseTexPath;
    Texture diffuseTex;

    Material(vec3 diffuseCol) { diffuseColor = diffuseCol; }
    Material(vec3 diffuseCol, const std::string& texName) { diffuseColor = diffuseCol; diffuseTexPath = texName; }
};


 
// https://github.com/canmom/rasteriser

void components_to_vec2s(const std::vector<f32> components, std::vector<vec2>& vecs) {
    //convert a vector of back-to-back vertex components to a vector of vec2 objects
    for (size_t vec_start = 0; vec_start < components.size(); vec_start+=2) {
        vecs.push_back(vec2(components[vec_start], components[vec_start+1]));
    }
}

void components_to_vec3s(const std::vector<f32> components, std::vector<vec3>& vecs) {
    //convert a vector of back-to-back vertex components to a vector of vec3 objects
    for (size_t vec_start = 0; vec_start < components.size(); vec_start+=3) {
        vec3 x = vec3(components[vec_start], components[vec_start+1], components[vec_start+2]);
        vecs.push_back(x);
    }
}

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

void load_triangles(const tinyobj::shape_t & shape, std::vector<Triangle> & triangles) {
    //convert a tinyobjloader shape_t object containing indices into vertex properties and textures
    //into a vector of Triangle objects grouping these indices
    const std::vector<tinyobj::index_t> & indices = shape.mesh.indices;
    const std::vector<int> & mat_ids = shape.mesh.material_ids;

    std::cout << "Loading " << mat_ids.size() << " triangles..." << std::endl;

    for(size_t face_ind = 0; face_ind < mat_ids.size(); face_ind++) {
        triangles.push_back(
            Triangle(
                {indices[3*face_ind].vertex_index, indices[3*face_ind+1].vertex_index, indices[3*face_ind+2].vertex_index},
                {indices[3*face_ind].normal_index, indices[3*face_ind+1].normal_index, indices[3*face_ind+2].normal_index},
                {indices[3*face_ind].texcoord_index, indices[3*face_ind+1].texcoord_index, indices[3*face_ind+2].texcoord_index},
                mat_ids[face_ind]
            ));
    }
}

void load_obj(
    const char* filename, const char* matsDirectory, 
    std::vector<Vertex>& vertices, std::vector<u32> indices,
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
        true, //enable triangulation
        false
    ); 

    //boilerplate error handling
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!success) {
        CloseGameWindow();
    }


    // ------ data coming in
    // attrib.
    //  vertices - array of floats representing vertices (triangulated, so in groups of 3)
    //  normals - same as above
    //  colors - same as above
    //  texcoords - same as above, but in groups of 2
    //
    // shape.mesh.indices
    //  normal_index
    //  vertex_index
    //  texcoord_index
    //
    // shape.mesh.material_ids
    //  array of s32's that represent material ids

    const std::vector<tinyobj::real_t>& attribVerts = attrib.vertices;
    const std::vector<tinyobj::real_t>& attribNorms = attrib.normals;
    const std::vector<tinyobj::real_t>& attribTexCoords = attrib.texcoords;
    const std::vector<tinyobj::real_t>& attribColors = attrib.colors;

    std::cout << attribVerts.size() << " " << attribNorms.size() << " " << attribTexCoords.size() << " " << attribColors.size() << "\n";


    for (const auto& shape : shapes) {
        const std::vector<tinyobj::index_t>& meshIndices = shape.mesh.indices;
        const std::vector<s32>& materialIds = shape.mesh.material_ids;
        // size of materialIds is size of meshIndices / 3
        for (s32 i = 0; i < meshIndices.size(); i++) {
            Vertex v = {};
            vec3 vertPos = vec3(
                attribVerts.at( meshIndices.at(i).vertex_index+0 ),
                attribVerts.at( meshIndices.at(i).vertex_index+1 ),
                attribVerts.at( meshIndices.at(i).vertex_index+2 )
            );
            v.position = vertPos;
            vec3 norm = vec3(
                attribNorms.at( meshIndices.at(i).normal_index+0 ),
                attribNorms.at( meshIndices.at(i).normal_index+1 ),
                attribNorms.at( meshIndices.at(i).normal_index+2 )
            );
            v.normal = norm;
            vec2 texCoord = vec2(
                attribTexCoords.at( meshIndices.at(i).texcoord_index+0 ),
                attribTexCoords.at( meshIndices.at(i).texcoord_index+1 )
            );
            v.texCoords = texCoord;
            vertices.push_back(v);
            // ???? what do i do with indices here
            indices.push_back(meshIndices.at(i).vertex_index);
        }
    }

    /*
    //convert the vertices into our format
    components_to_vec3s(attrib.vertices, vertices);

    //convert the vertex normals into our format
    components_to_vec3s(attrib.normals, vertnormals);

    //convert the uv coordinates into our format
    components_to_vec2s(attrib.texcoords, vertuvs);

    //convert the face_inds into our format
    //face_inds should all be triangles due to triangulate=true
    for (auto shape : shapes) {
        load_triangles(shape, triangles);
    }
    */

    //conver materials and load textures
    load_materials(objmaterials, matsDirectory, materials);

    std::cout << "Loaded model " << filename << "." << std::endl;
}
/*
void load_obj(
    const char* filename, const char* matsDirectory, 
    std::vector<vec3> &vertices, std::vector<Triangle> &triangles, 
    std::vector<vec3> & vertnormals, std::vector<vec2>& vertuvs, 
    std::vector<Material>& materials) {
    //load a Wavefront .obj file at 'file' and store vertex coordinates as vec3 and face_inds as uvec3 of indices

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objmaterials;
    std::string err; std::string warn;
    
    //load all data in Obj file & 'triangulate'
    bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, 
        &warn, &err,
        filename, //model to load
        matsDirectory, //directory to search for materials
        true, //enable triangulation
        false
    ); 

    //boilerplate error handling
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!success) {
        CloseGameWindow();
    }

    
    //convert the vertices into our format
    components_to_vec3s(attrib.vertices, vertices);

    //convert the vertex normals into our format
    components_to_vec3s(attrib.normals, vertnormals);

    //convert the uv coordinates into our format
    components_to_vec2s(attrib.texcoords, vertuvs);

    //conver materials and load textures
    load_materials(objmaterials, matsDirectory, materials);

    //convert the face_inds into our format
    //face_inds should all be triangles due to triangulate=true
    for (auto shape : shapes) {
        load_triangles(shape, triangles);
    }


    std::cout << "Loaded model " << filename << "." << std::endl;
}*/

#endif