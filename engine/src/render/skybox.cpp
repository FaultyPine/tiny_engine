//#include "pch.h"
#include "skybox.h"
#include "tiny_ogl.h"
#include "tiny_log.h"
#include "tiny_profiler.h"

static constexpr f32 skyboxVertices[] = 
{
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

static Mesh skyboxCube;

void InitSkybox(Skybox* skybox)
{
    if (!skybox->skyboxShader.isValid()) {
        skybox->skyboxShader = Shader(ResPath("shaders/skybox.vert"), ResPath("shaders/skybox.frag"));
        skybox->skyboxShader.TryAddSampler(skybox->cubemap, "skybox");
    }
    if (!skyboxCube.isValid())
    {
        std::vector<Vertex> vertices = {};
        for (u32 i = 0; i < ARRAY_SIZE(skyboxVertices); i+=3) {
            Vertex v = {};
            v.position = glm::vec3(skyboxVertices[i+0],skyboxVertices[i+1],skyboxVertices[i+2]);
            vertices.push_back(v);
        }
        skyboxCube = Mesh(vertices, {}, {}, "Skybox");
    }
}

Skybox::Skybox()
{
    InitSkybox(this);
}

Skybox::Skybox(const std::vector<const char*>& facesPaths, TextureProperties props) 
{
    PROFILE_FUNCTION();
    cubemap = LoadCubemap(facesPaths, props);
    InitSkybox(this);
}


void Skybox::Draw() const
{
    PROFILE_FUNCTION();
    PROFILE_FUNCTION_GPU();
    if (!skyboxShader.isValid())
    {
        LOG_WARN("Skybox shader invalid while drawing");
        return;
    }
    LightDirectional& sun = GetEngineCtx().lightsSubsystem->lights.sunlight;
    UpdateSunlightValues(skyboxShader, sun);
    glDepthFunc(GL_LEQUAL);
    skyboxShader.use();
    skyboxCube.Draw();
    glDepthFunc(GL_LESS);
}