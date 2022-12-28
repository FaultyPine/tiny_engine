#include "model.h"

#include "ObjParser.h"
#include "camera.h"
#include "tiny_engine/math.h"
#include "tiny_engine/tiny_engine.h"


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

void Model::Draw(const Shader& shader, const Transform& tf, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        glm::mat4 model = Math::Position3DToModelMat(tf.position, tf.scale, tf.rotation, tf.rotationAxis);
        shader.setUniform("modelMat", model);
        glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(model)));
        shader.setUniform("normalMat", matNormal);
        shader.setUniform("time", GetTimef());

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, tf);
    }
}
void Model::Draw(const Shader& shader, const glm::mat4& mvp, const glm::mat4& modelMat, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        shader.setUniform("modelMat", modelMat);
        glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(modelMat)));
        shader.setUniform("normalMat", matNormal);
        shader.setUniform("time", GetTimef());

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, mvp);
    }
}


void Model::DrawInstanced(const Shader& shader, const std::vector<Transform>& transforms, const std::vector<Light>& lights) const {
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        // set model-space matrix seperately so we can get fragment WS positions and WS normals
        //glm::mat4 model = Math::Position3DToModelMat(tf.position, tf.scale, tf.rotation, tf.rotationAxis);
        //shader.setUniform("modelMat", model);
        //glm::mat3 matNormal = glm::mat3(glm::transpose(glm::inverse(model)));
        //shader.setUniform("normalMat", matNormal);
        shader.setUniform("time", GetTimef());

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.DrawInstanced(shader, transforms);
    }
}
