#include "model.h"

#include "ObjParser.h"
#include "camera.h"
#include "tiny_engine/math.h"


Model::Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir) {
    meshes = LoadObjMesh(meshObjFile, meshMaterialDir);
    cachedShader = shader;
}
void Model::Draw(glm::vec3 pos, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis, const std::vector<Light>& lights) const {
    for (const Mesh& mesh : meshes) {
        cachedShader.use();
        cachedShader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(pos, glm::vec3(scale), rotation, rotationAxis);
        cachedShader.setUniform("modelMat", model);

        for (const Light& light : lights)
            UpdateLightValues(cachedShader, light);

        mesh.Draw(cachedShader, pos, scale, rotation, rotationAxis);
    }
}
void Model::Draw(const Shader& shader, glm::vec3 pos, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis, const std::vector<Light>& lights) const {
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(pos, scale, rotation, rotationAxis);
        shader.setUniform("modelMat", model);

        for (const Light& light : lights)
            UpdateLightValues(shader, light);

        mesh.Draw(shader, pos, scale, rotation, rotationAxis);
    }
}
void Model::Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights) const {
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        shader.setUniform("modelMat", modelMat);

        for (const Light& light : lights)
            UpdateLightValues(shader, light);

        mesh.Draw(shader, mvp);
    }
}
