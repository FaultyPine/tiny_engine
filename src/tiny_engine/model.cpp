#include "model.h"

#include "ObjParser.h"
#include "camera.h"

Model::Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir) {
    meshes = LoadObjMesh(shader, meshObjFile, meshMaterialDir);
}
void Model::AddLight(const Light& light) {
    isLit = true;
    lights.push_back(light);
    assert(lights.size() < MAX_NUM_LIGHTS);
}
void Model::Draw(glm::vec3 pos, f32 scale, f32 rotation, glm::vec3 rotationAxis) {
    for (Mesh& mesh : meshes) {
        Shader& shader = mesh.GetShader();
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);

        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(rotation), rotationAxis); 
        model = glm::translate(model, pos);
        model = glm::scale(model, glm::vec3(scale));
        shader.setUniform("modelMat", model);

        if (isLit) {
            for (Light& light : lights)
                UpdateLightValues(shader, light);
        }

        mesh.Draw(pos, scale, rotation, rotationAxis);
    }
}
