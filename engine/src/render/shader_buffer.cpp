#include "shader_buffer.h"

#include "tiny_ogl.h"
#include "shader.h"
#include "tiny_engine.h"
#include "camera.h"
#include "scene/entity.h"
#include "tiny_profiler.h"

#include <set>

// only have 1 big UBO
#define UBO_BINDING_POINT 0
#define UBO_NAME "Globals"


void GetUBOGlobalsCamera(const Camera& cam, UBOGlobals& globs)
{
    // populate uniform block structure. Need to convert from cam to this to adhere to 
    // the uniform block memory layout
    globs.camFront = glm::vec4(cam.cameraFront, 0.0);
    globs.camPos = glm::vec4(cam.cameraPos, 0.0);
    globs.farClip = cam.farClip;
    globs.nearClip = cam.nearClip;
    globs.FOV = cam.FOV;
    globs.projection = cam.GetProjectionMatrix();
    globs.view = cam.GetViewMatrix();
}


// ----- Updating UBO data... contains camera-specific, lighting specific, etc
void UpdateGlobalUBOCamera(UBOGlobals& globs)
{
    GetUBOGlobalsCamera(Camera::GetMainCamera(), globs);
}

void UpdateGlobalUBOLighting(UBOGlobals& globs)
{
    EngineContext& ctx = GetEngineCtx();
    GlobalLights& lights = ctx.lightsSubsystem->lights;
    s32 lightIdx = 0;
    for (s32 i = 0; i < MAX_NUM_LIGHTS; i++)
    {
        if (lights.pointLights->enabled)
        {
            const LightPoint& pointLight = lights.pointLights[i];
            globs.lights[lightIdx].position = glm::vec4(pointLight.position, 0.0);
            globs.lights[lightIdx].color = pointLight.color;
            globs.lights[lightIdx].attenuationParams = glm::vec4(pointLight.constant, pointLight.linear, pointLight.quadratic, pointLight.intensity);
            lightIdx++;
        }
    }
    globs.sunlight.color = lights.sunlight.color;
    globs.sunlight.direction = glm::vec4(lights.sunlight.direction, lights.sunlight.intensity);
    globs.sunlight.lightSpaceMatrix = lights.sunlight.GetLightSpacematrix();

    globs.numActiveLightsAndAmbientIntensity.x = (f32)lights.GetNumActiveLights();
    globs.numActiveLightsAndAmbientIntensity = glm::vec4(globs.numActiveLightsAndAmbientIntensity.x, ctx.lightsSubsystem->ambientLightIntensity, 0.0f, 0.0f);
}

void UpdateGlobalUBOMisc(UBOGlobals& globs)
{
    globs.time = GetTimef();
    globs.screenSize = glm::vec4(Camera::GetScreenWidth(), Camera::GetScreenHeight(), 0.0f, 0.0f);
}
// -------------------------

void InitializeUBOs(ShaderBufferGlobals& globals)
{
    PROFILE_FUNCTION();
    PROFILE_FUNCTION_GPU();
    u32& uboObject = globals.uboObject;
    glGenBuffers(1, &uboObject);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uboObject);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(UBOGlobals), NULL, GL_DYNAMIC_DRAW); // allocate vmem
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UBO_BINDING_POINT, uboObject); // never binding a different one rn, so this stays bound always
}

bool TryBindUniformBlockToBindingPoint(const char* uniformBlockName, u32 bindingPoint, u32 shaderProgram)
{
    //u32 blockIndex = glGetUniformBlockIndex(shaderProgram, uniformBlockName);
    u32 blockIndex = glGetProgramResourceIndex(shaderProgram, GL_SHADER_STORAGE_BLOCK, uniformBlockName);
    if (blockIndex == GL_INVALID_INDEX) 
    {
        //LOG_ERROR("Failed to get uniform block index!");
        return false;
    }
    else
    {
        //GLCall(glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint));
        GLCall(glShaderStorageBlockBinding (shaderProgram, blockIndex, bindingPoint));
    }
    return true;
}

void UpdateGlobalUBOModelMatrices(UBOGlobals& globs)
{
    PROFILE_FUNCTION();
    EngineContext& ctx = GetEngineCtx();
    u32 numRenderableEntities = 0;
    Entity::GetRenderableEntities(nullptr, &numRenderableEntities);
    EntityRef* renderableEntites = arena_alloc_type(&ctx.engineArena, EntityRef, numRenderableEntities);
    Entity::GetRenderableEntities(renderableEntites, &numRenderableEntities);
    std::set<u32> ensureUniqueIdxs = {};
    for (u32 i = 0; i < numRenderableEntities; i++)
    {
        EntityRef ent = renderableEntites[i];
        const EntityData& entityData = Entity::GetEntity(ent);
        u32 idx = (u32)ent % ARRAY_SIZE(globs.objectData);
        ensureUniqueIdxs.insert(idx);
        glm::mat4 model = entityData.transform.ToModelMatrix();
        glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model)));
        UBOGlobals::GPUPerObjectData& objData = globs.objectData[idx];
        objData.modelMat = model;
        objData.normalMat = normal;
    }
    // if this hits, we have a hash collision. Each index into our gpu buffer should be unique.
    TINY_ASSERT(ensureUniqueIdxs.size() == numRenderableEntities);
    arena_pop_latest(&ctx.engineArena, renderableEntites);
}

void ShaderSystemPreDraw(ShaderBufferGlobals& globals)
{
    PROFILE_FUNCTION();
    PROFILE_FUNCTION_GPU();
    UBOGlobals& globs = globals.globals;
    TMEMSET(&globs, 0, sizeof(UBOGlobals));
    UpdateGlobalUBOCamera(globs);
    UpdateGlobalUBOLighting(globs);
    // materials set per shader in uniform. uniform samplers cant be sampled by dynamic index, and doing crazy 3d texture hacks doesn't seem worth it rn
    //UpdateGlobalUBOMaterials(globs); 
    UpdateGlobalUBOMisc(globs);
    UpdateGlobalUBOModelMatrices(globs);

    // update the entire ubo block every frame
    if (!globals.uboObject) 
    {
        LOG_FATAL("UBO's not initialized. Make sure InitializeShaderSystem is being called");
        return;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, globals.uboObject);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(UBOGlobals), &globals.globals);
}

void HandleShaderUBOInit(u32 shaderProgram)
{
    // shaders that don't use the ubo won't have it bound
    bool hasUBO = TryBindUniformBlockToBindingPoint(UBO_NAME, UBO_BINDING_POINT, shaderProgram);
}
