//#include "pch.h"
#include "skybox.h"
#include "tiny_ogl.h"
#include "tiny_log.h"

static f32 skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

static Shader skyboxShader;
static Mesh skyboxCube;

Skybox::Skybox(const std::vector<const char*>& facesPaths, TextureProperties props) {
    cubemap = LoadCubemap(facesPaths, props);
    if (!skyboxShader.isValid() && !skyboxCube.isValid()) {
        skyboxShader = Shader(ResPath("shaders/skybox.vert"), ResPath("shaders/skybox.frag"));
        skyboxShader.TryAddSampler(cubemap, "skybox");
        std::vector<Vertex> vertices = {};
        for (u32 i = 0; i < ARRAY_SIZE(skyboxVertices); i+=3) {
            Vertex v = {};
            v.position = glm::vec3(skyboxVertices[i+0],skyboxVertices[i+1],skyboxVertices[i+2]);
            vertices.push_back(v);
        }
        skyboxCube = Mesh(vertices, {}, {}, "Skybox");
    }
}


void Skybox::Draw() {
    if (!skyboxShader.isValid())
    {
        LOG_WARN("Skybox shader invalid while drawing");
        return;
    }
    LightDirectional& sun = GetEngineCtx().lightsSubsystem->sunlight;
    UpdateSunlightValues(skyboxShader, sun);
    GLCall(glDepthFunc(GL_LEQUAL));
    glm::mat4 view = Camera::GetMainCamera().GetViewMatrix();
    // remove translation from the view matrix
    // This centers the cube on the camera since the vertices are [-1, 1]
    view = glm::mat4(glm::mat3(view)); 
    skyboxShader.setUniform("view", view);
    skyboxShader.setUniform("projection", Camera::GetMainCamera().GetProjectionMatrix());
    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id));
    skyboxCube.Draw(skyboxShader);
    GLCall(glDepthFunc(GL_LESS));
}