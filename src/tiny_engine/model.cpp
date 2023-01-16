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


BoundingBox Model::GetBoundingBox() {
    BoundingBox bounds = {};

    if (!this->meshes.empty()) {
        glm::vec3 temp = glm::vec3(0);
        bounds = this->meshes[0].GetMeshBoundingBox();

        for (s32 i = 1; i < this->meshes.size(); i++) {
            BoundingBox tempBounds = this->meshes[i].GetMeshBoundingBox();

            // get min for each component
            temp.x = (bounds.min.x < tempBounds.min.x) ? bounds.min.x : tempBounds.min.x;
            temp.y = (bounds.min.y < tempBounds.min.y) ? bounds.min.y : tempBounds.min.y;
            temp.z = (bounds.min.z < tempBounds.min.z) ? bounds.min.z : tempBounds.min.z;
            bounds.min = temp;

            // get max for each component
            temp.x = (bounds.max.x > tempBounds.max.x) ? bounds.max.x : tempBounds.max.x;
            temp.y = (bounds.max.y > tempBounds.max.y) ? bounds.max.y : tempBounds.max.y;
            temp.z = (bounds.max.z > tempBounds.max.z) ? bounds.max.z : tempBounds.max.z;
            bounds.max = temp;
        }
    }

    return bounds;
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

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.Draw(shader, mvp);
    }
}


void Model::DrawInstanced(const Shader& shader, u32 numInstances, const std::vector<Light>& lights) const {
    // this is hardcoded in the (grass) shader right now. 
    // TODO: refactor instancing to put instance-relevant data in a vertex attribute
    // instead of relying on gl_InstanceID to index into big uniform arrays
    ASSERT(AreActiveLightsInFront(lights));
    for (const Mesh& mesh : meshes) {
        shader.use();
        Camera& cam = Camera::GetMainCamera();
        glm::mat4 view = cam.GetViewMatrix();
        glm::mat4 projection = cam.GetProjectionMatrix();

        shader.setUniform("viewMat", view);
        shader.setUniform("projectionMat", projection);
        shader.setUniform("viewPos", cam.cameraPos);

        for (const Light& light : lights) {
            if (light.enabled)
                UpdateLightValues(shader, light);
        }
        shader.setUniform("numActiveLights", (s32)lights.size());

        mesh.DrawInstanced(shader, numInstances);
    }
}

Mesh* Model::GetMesh(const std::string& name) {
    for (Mesh& mesh : meshes) {
        if (mesh.name == name) return &mesh;
    }
    return nullptr;
}