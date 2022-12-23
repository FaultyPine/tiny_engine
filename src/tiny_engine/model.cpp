#include "model.h"

#include "ObjParser.h"
#include "camera.h"
#include "tiny_engine/math.h"


Model::Model(const Shader& shader, const char* meshObjFile, const char* meshMaterialDir) {
    meshes = LoadObjMesh(meshObjFile, meshMaterialDir);
    cachedShader = shader;
}
Model::Model(const Shader& shader, const std::vector<Mesh>& meshes) {
    cachedShader = shader;
    this->meshes = meshes;
}

bool AreActiveLightsInFront(const std::vector<Light>& lights) {
    for (u32 i = 0; i < lights.size(); i++) {
        if (!lights.at(i).enabled) {
            for (u32 j = i+1; j < lights.size(); j++) {
                if (lights.at(i).enabled) {
                    return false;
                }
            }
        }
    }
    return true;
}

void Model::Draw(glm::vec3 pos, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        cachedShader.use();
        cachedShader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(pos, glm::vec3(scale), rotation, rotationAxis);
        cachedShader.setUniform("modelMat", model);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(cachedShader, light);
        }
        cachedShader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(cachedShader, pos, scale, rotation, rotationAxis);
    }
}
void Model::Draw(const Shader& shader, glm::vec3 pos, glm::vec3 scale, f32 rotation, glm::vec3 rotationAxis, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(pos, scale, rotation, rotationAxis);
        shader.setUniform("modelMat", model);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, pos, scale, rotation, rotationAxis);
    }
}
void Model::Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        shader.setUniform("modelMat", modelMat);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, mvp);
    }
}
