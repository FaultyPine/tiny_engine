#include "shader_buffer.h"

#include "tiny_ogl.h"
#include "shader.h"
#include "tiny_engine.h"
#include "camera.h"

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

void UpdateGlobalUBOMaterials(UBOGlobals& globs)
{
    // TODO:
    MaterialRegistry& matRegistry = *GetEngineCtx().materialRegistry;
    
}

void UpdateGlobalUBOMisc(UBOGlobals& globs)
{
    globs.time = GetTimef();
}
// -------------------------

void InitializeUBOs(ShaderBufferGlobals& globals)
{
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

void ShaderSystemPreDraw(ShaderBufferGlobals& globals)
{
    UBOGlobals& globs = globals.globals;
    TMEMSET(&globs, 0, sizeof(UBOGlobals));
    UpdateGlobalUBOCamera(globs);
    UpdateGlobalUBOLighting(globs);
    UpdateGlobalUBOMaterials(globs);
    UpdateGlobalUBOMisc(globs);

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
