//#include "pch.h"
#include "testbed_main.h"

#include "tiny_imgui.h"

// =============== testbed_main.h ========================
#include "render/model.h"
#include "tiny_types.h"
#include "tiny_engine.h"
#include "render/sprite.h"
#include "render/framebuffer.h"
#include "render/skybox.h"
#include "particles/particles.h"
#include "mem/tiny_arena.h"
#include "physics/tiny_physics.h"
#include "render/tiny_renderer.h"
#include "scene/entity.h"
#include "render/tiny_ogl.h"
#include "render/postprocess.h"

//#define ISLAND_SCENE
#define SPONZA_SCENE


struct Wave {
    f32 waveSpeed = 1.0;
    f32 wavelength = 10.0;
    f32 steepness = 0.3;
    glm::vec2 direction = glm::vec2(1,0);
    Wave(f32 ws, f32 wl, f32 stp, glm::vec2 dir)
        : waveSpeed(ws), wavelength(wl), steepness(stp), direction(dir) {}
    Wave(){}
};

inline u32 GetHash(const std::string& str) {
    return HashBytes((u8*)str.data(), str.size());
}

struct GameState {
    Arena* gameMem;

    std::vector<EntityRef> entities = {};
    Shader lightingShader = {};

    // Main pond
    #define NUM_WAVES 8
    Wave waves[NUM_WAVES];
    s32 numActiveWaves = 0;
    Texture waterTexture;
    // Waterfall particles
    ParticleSystem waterfallParticles = {};
    
    // grass
    BoundingBox grassSpawnExclusion = {};
    Texture windTexture = {};
    f32 windStrength = 0;
    f32 windFrequency = 0;
    f32 windUVScale = 0;
    f32 grassCurveIntensity = 0;

    Skybox skybox = {};

    f32 sunOrbitRadius = 52.0f;
    f32 sunSpeedMultiplier = 0.2f;
    glm::vec3 sunTarget = glm::vec3(0, 0, 0);

    // sets the static gamestate to a copy of the passed in gamestate
    static GameState* Initialize(Arena* gameMem)
    {
        GameState* newGamestate = (GameState*)arena_alloc(gameMem, sizeof(GameState));
        *newGamestate = {}; // default init
        newGamestate->gameMem = gameMem;
        return newGamestate;
    }
    void Terminate() {
        for (auto& ent : entities) {
            Entity::DestroyEntity(ent);
        }
    }

};
// ===========================================================


#include "camera.h"
#include "input.h"
#include "render/shader.h"
#include "tiny_fs.h"
#include "render/mesh.h"
#include "render/texture.h"
#include "render/tiny_lights.h"
#include "math/tiny_math.h"
#include "render/shapes.h"
#include "tiny_profiler.h"
#include "particles/particle_behaviors.h"
#include "tiny_log.h"
#include "xatlas/xatlas.h"


void testbed_camera_tick() {
    Camera& cam = Camera::GetMainCamera();
    f32 cameraSpeed = cam.speed * GetDeltaTime();
    if (Keyboard::isKeyDown(TINY_KEY_LEFT_CONTROL))
    {
        cameraSpeed *= 15.0f;
    }
    glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
    if (Keyboard::isKeyDown(TINY_KEY_W)) {
        cam.cameraPos += cameraSpeed * glm::vec3(cam.cameraFront.x, 0.0, cam.cameraFront.z);
    }
    if (Keyboard::isKeyDown(TINY_KEY_S)) {
        cam.cameraPos -= cameraSpeed * glm::vec3(cam.cameraFront.x, 0.0, cam.cameraFront.z);
    }
    if (Keyboard::isKeyDown(TINY_KEY_A)) {
        cam.cameraPos -= cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(TINY_KEY_D)) {
        cam.cameraPos += cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(TINY_KEY_SPACE)) {
        cam.cameraPos.y += cameraSpeed;
    }
    if (Keyboard::isKeyDown(TINY_KEY_LEFT_SHIFT)) {
        cam.cameraPos.y -= cameraSpeed;
    }
    if (Keyboard::isKeyDown(TINY_KEY_ESCAPE)) {
        CloseGameWindow();
    }


    if (Keyboard::isKeyPressed(TINY_KEY_R)) {
        Shader::ReloadShaders();
    }


}

void testbed_orbit_cam(f32 orbitRadius, f32 cameraOrbitHeight, glm::vec3 lookAtPos) {
    Camera& cam = Camera::GetMainCamera();
    f32 time = (f32)GetTime();
    f32 x = sinf(time) * orbitRadius;
    f32 z = cosf(time) * orbitRadius;
    cam.cameraPos = glm::vec3(x, cameraOrbitHeight, z);
    cam.LookAt(lookAtPos);
}
void testbed_orbit_light(LightDirectional& light, f32 orbitRadius, f32 speedMultiplier, glm::vec3 target) {
    f32 time = (f32)GetTime()*speedMultiplier;
    f32 x = sinf(time) * orbitRadius;
    f32 z = cosf(time) * orbitRadius;
    //f32 x = sinf(time);
    //f32 z = cosf(time);
    light.position.x = x;
    light.position.z = z;
    light.direction = glm::normalize(target - light.position);
}

static bool enableGrassRender = true;
static f32 ambientLightIntensity = 0.15f;
void drawImGuiDebug(GameState& gs) {
    PROFILE_FUNCTION();
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    glm::vec3 camPos = Camera::GetMainCamera().cameraPos;
    ImGui::Text(TextFormat("CamPos: %f %f %f", camPos.x, camPos.y, camPos.z));

    if (ImGui::CollapsingHeader("Entities"))
    {
        for (EntityRef& entref : gs.entities)
        {
            EntityData& ent = Entity::GetEntity(entref);
            const char* entityLabel = TextFormat("Position [%s]", ent.name);
            ImGui::DragFloat3(entityLabel, &ent.transform.position[0]);
        }
    }
    if (ImGui::Checkbox("Enable grass render", &enableGrassRender))
    {
        Entity::SetFlag(GetHash("grass"), EntityFlags::DISABLED, !enableGrassRender);
    }

    PostprocessSettings& ppSettings = Postprocess::ModifySettings();
    ImGui::DragFloat("SSAO power", &ppSettings.ssaoSettings.occlusionPower, 0.05, 0.0, 10.0);
    ImGui::DragFloat("SSAO sample radius", &ppSettings.ssaoSettings.sampleRadius, 0.05, 0.0, 1.0);

    ImGui::DragFloat("wind str", &gs.windStrength, 0.01f);
    ImGui::DragFloat("wind freq", &gs.windFrequency, 0.01f);
    ImGui::DragFloat("wind uvscale", &gs.windUVScale, 0.01f);
    ImGui::DragFloat("grass curve intensity", &gs.grassCurveIntensity, 0.1f);
    if (ImGui::CollapsingHeader("Grass spawn planes")) {
        ImGui::DragFloat3("Grass ex min", &gs.grassSpawnExclusion.min[0], 0.01f);
        ImGui::DragFloat3("Grass ex max", &gs.grassSpawnExclusion.max[0], 0.01f);
    }
    if (EntityData& waveEntity = Entity::GetEntity("PondEntity")) {
        if (ImGui::CollapsingHeader("Main Pond") && waveEntity.isValid()) {
            if (ImGui::CollapsingHeader("Wave Plane")) {
                //ImGui::DragFloat3("Wave pos", &waveEntity.transform.position[0], 0.01f);
                //ImGui::DragFloat3("Wave scale", &waveEntity.transform.scale[0], 0.01f);
            }
            for (u32 i = 0; i < NUM_WAVES; i++) {
                Wave& wave = gs.waves[i];
                if (ImGui::CollapsingHeader(TextFormat("Wave %i", i))) {
                    //ImGui::Text("Wave %i", i);
                    ImGui::DragFloat("Speed", &wave.waveSpeed, 0.01f);
                    ImGui::DragFloat("Wavelength", &wave.wavelength, 0.01f);
                    ImGui::DragFloat("Steepness", &wave.steepness, 0.01f);
                    ImGui::DragFloat2("Direction", &wave.direction[0], 0.1f);
                }
            }
        }
    }

    ImGui::DragFloat("Ambient light intensity", &GetEngineCtx().lightsSubsystem->ambientLightIntensity, 0.001f);
    if (ImGui::CollapsingHeader("Point lights"))
    {
        for (u32 i = 0; i < MAX_NUM_LIGHTS; i++)
        {
            LightPoint& light = GetEngineCtx().lightsSubsystem->lights.pointLights[i];
            const char* lightLabel = TextFormat("Light %i", i);
            if (ImGui::CollapsingHeader(lightLabel))
            {
                ImGui::ColorEdit4("Light Color", &light.color[0]);
                ImGui::DragFloat3("Light pos", &light.position[0], 0.1f);
                ImGui::DragFloat("Light intensity", &light.intensity, 0.01f);
                ImGui::DragFloat3("Attenuation params", &light.constant, 0.01f);
                ImGui::Checkbox("Enabled", (bool*)&light.enabled);
            }
        }
    }
    LightDirectional& sunlight = GetEngineCtx().lightsSubsystem->lights.sunlight;
    if (ImGui::CollapsingHeader("Sunlight"))
    {
        if (ImGui::Button("Teleport to sun"))
        {
            Camera& cam = Camera::GetMainCamera();
            cam.cameraPos = sunlight.position;
            cam.LookAt(sunlight.position + sunlight.direction);
        }
        ImGui::ColorEdit4("sunlight Color", &sunlight.color[0]);
        ImGui::DragFloat3("sunlight direction", &sunlight.direction[0], 0.1f);
        sunlight.direction = glm::normalize(sunlight.direction);
        ImGui::DragFloat3("sunlight position", &sunlight.position[0], 0.1f);
        ImGui::DragFloat("sunlight intensity", &sunlight.intensity, 0.01f);
        ImGui::Checkbox("sunlight Enabled", (bool*)&sunlight.enabled);     
        ImGui::DragFloat("Sun Orbit radius", &gs.sunOrbitRadius);
        ImGui::DragFloat("Sun orbit speed", &gs.sunSpeedMultiplier, 0.01f);
        ImGui::DragFloat3("Sun target", &gs.sunTarget[0]);

        glm::mat4 proj, view;
        sunlight.GetLightSpacematrix(&proj, &view);
        Renderer::PushFrustum(proj, view);
    }

}


glm::vec3 RandomPointBetweenVertices(const std::vector<Vertex>& planeVerts) {
    // pick two verts and pick a random spot between those two points to spawn it in
    u32 randPointIdx1 = GetRandom(0, planeVerts.size());
    u32 randPointIdx2 = GetRandom(0, planeVerts.size());
    while (randPointIdx2 == randPointIdx1) {
        // don't pick two of the same vertex
        randPointIdx2 = GetRandom(0, planeVerts.size());
    }
    const Vertex& vert1 = planeVerts.at(randPointIdx1);
    const Vertex& vert2 = planeVerts.at(randPointIdx2);
    // after picking two random vertices, pick a random amount [0,1] and lerp between those to find this "random" grass spawn position
    glm::vec3 point = Math::Lerp(vert1.position, vert2.position, GetRandomf(0.0f, 1.0f));
    return point;
}

void PopulateGrassTransformsFromSpawnPlane(
    const BoundingBox& spawnExclusion, 
    const std::vector<Vertex>& planeVerts, 
    glm::mat4* grassTransforms, 
    u32 numGrassInstancesToSpawn) {
    PROFILE_FUNCTION();
    constexpr f32 spawnNeighborLeniency = 0.5f;
    constexpr f32 grassSpawnHeight = 7.6; // ew hardcoded
    glm::vec2 exclusionMin = glm::vec2(spawnExclusion.min.x, spawnExclusion.min.z);
    glm::vec2 exclusionMax = glm::vec2(spawnExclusion.max.x, spawnExclusion.max.z);
    for (u32 i = 0; i < numGrassInstancesToSpawn; i++) {
        glm::vec2 randomPointBetweenVerts = glm::vec2(0);
        // pick random point. If point is within "excluded" region, pick another point
        do {
            glm::vec3 point = RandomPointBetweenVertices(planeVerts);
            randomPointBetweenVerts = glm::vec2(point.x, point.z);
        } while (Math::isPointInRectangle(randomPointBetweenVerts, exclusionMin, exclusionMax));
        f32 randRotation = GetRandomf(0.0f, 360.0f);
        f32 randScale = GetRandomf(0.3f, 0.7f);
        Transform grassTransform = Transform(glm::vec3(randomPointBetweenVerts.x, grassSpawnHeight, randomPointBetweenVerts.y), glm::vec3(randScale), randRotation);
        grassTransforms[i] = grassTransform.ToModelMatrix();
    }
}

void init_grass(GameState& gs) {
    PROFILE_FUNCTION();
    // area where grass CANNOT spawn
    gs.grassSpawnExclusion = BoundingBox({-5.39, 8, -0.43}, 
                                         {6.67, 8, 6.9});
    constexpr u32 NUM_GRASS_BLADES = 100000;
    LOG_INFO("Allocating %llu", sizeof(glm::mat4) * NUM_GRASS_BLADES);
    glm::mat4* grassTransforms = arena_alloc_type(gs.gameMem, glm::mat4, NUM_GRASS_BLADES);
    //  init grass transforms
    EntityData& islandModel = Entity::GetEntity("island");
    if (islandModel) 
    {
        Mesh* grassSpawnMesh = islandModel.model.GetMesh("GrassSpawnPlane_Mesh");
        if (grassSpawnMesh) 
        {
            grassSpawnMesh->isVisible = false;
            PopulateGrassTransformsFromSpawnPlane(gs.grassSpawnExclusion, grassSpawnMesh->vertices, grassTransforms, NUM_GRASS_BLADES);
        }
    }

    Shader grassShader = Shader(ResPath("shaders/grass.vert").c_str(), ResPath("shaders/grass.frag").c_str());
    Transform grassTf = Transform({0,0,0}, {1,1,1});
    EntityRef grass = Entity::CreateEntity("grass", grassTf);
    gs.entities.push_back(grass);
    Model grassModel = Model(grassShader, 
                            ResPath("other/island_wip/grass_blade.obj").c_str(), 
                            ResPath("other/island_wip/").c_str(), 
                            grass);
    grassModel.EnableInstancing(grassTransforms, sizeof(glm::mat4), NUM_GRASS_BLADES);
    Entity::AddRenderable(grass, grassModel);
    PhysicsAddModel(grassModel, grassTf);
    gs.windTexture = LoadTexture(ResPath("other/distortion.png"));
    gs.windStrength = 0.15;
    gs.windFrequency = 1.1;
    gs.windUVScale = 0.15;
    gs.grassCurveIntensity = 1.0;
}

void init_main_pond(GameState& gs) {
    Shader waterShader = Shader(ResPath("shaders/water.vert").c_str(), ResPath("shaders/water.frag").c_str());
    gs.waterTexture = LoadTexture(ResPath("other/water.png"));
    waterShader.TryAddSampler(gs.waterTexture, "waterTexture");
    Transform waterPlaneTf = Transform({0.35, 3.64, 1.1}, {3.68, 1.0, 3.44});
    EntityRef waterPlaneEnt = Entity::CreateEntity("PondEntity", waterPlaneTf);
    Model waterPlane = Model(waterShader, {Shapes3D::GenPlaneMesh(30)}, waterPlaneEnt);
    Entity::AddRenderable(waterPlaneEnt, waterPlane);
    gs.entities.push_back(waterPlaneEnt);
    PhysicsAddModel(waterPlane, waterPlaneTf);
    gs.waves[0] = Wave(0.2, 8.7, 0.05, glm::vec2(1,1));
    gs.waves[1] = Wave(0.5, 2.0, 0.09, glm::vec2(0,1));
    gs.waves[2] = Wave(0.8, 1.0, 0.1, glm::vec2(1,0.4));
    gs.numActiveWaves = 3;
}

void init_waterfall(GameState& gs) {
    Shader waterfallShader = Shader(ResPath("shaders/waterfall.vert"), ResPath("shaders/waterfall.frag"));
    Texture waterfallTex = LoadTexture(ResPath("noise.jpg"));
    waterfallShader.TryAddSampler(waterfallTex, "waterfallTex");

    Transform waterfallTf = Transform({0, 0, 0}, {1.0, 1.0, 1.0});
    EntityRef waterfallEnt = Entity::CreateEntity("Waterfall", waterfallTf);
    Model waterfallModel = Model(waterfallShader, 
                                ResPath("other/island_wip/waterfall.obj").c_str(), 
                                ResPath("other/island_wip/").c_str(), 
                                waterfallEnt);
    Entity::AddRenderable(waterfallEnt, waterfallModel);
    gs.entities.push_back(waterfallEnt);
    PhysicsAddModel(waterfallModel, waterfallTf);
}


void testbed_init(Arena* gameMem) {
    PROFILE_FUNCTION();
    GameState& gs = *GameState::Initialize(gameMem);

    Camera::GetMainCamera().cameraPos.y = 10;
    // lol this should go away. Since renderer handles lit shader stuff, we should only give models a shader
    // if we want some custom behavior. No shader = reasonable default = standard lit.
    // can specify custom shader & flags like lit or unlit 
    // lit would only mean your custom shader is passed some lighting/material data
    gs.lightingShader = Shader(ResPath("shaders/default.vert"), ResPath("shaders/default.frag"));
    Shader lightingShader = gs.lightingShader;

#ifdef ISLAND_SCENE
    EntityRef islandEntRef = Entity::CreateEntity("island", Transform({0,0,0}));
    Model testModel = Model(lightingShader, 
                            ResPath("other/island_wip/island.obj").c_str(), 
                            ResPath("other/island_wip/").c_str(), 
                            islandEntRef);
    Entity::AddRenderable(islandEntRef, testModel);
    gs.entities.push_back(islandEntRef);
    EntityData& islandEnt = Entity::GetEntity(islandEntRef);
    PhysicsAddModel(testModel, islandEnt.transform);

    EntityRef treeEntRef = Entity::CreateEntity("tree", Transform({10,7.5,3}, glm::vec3(0.7)));
    Model treeModel = Model(lightingShader, 
                            ResPath("other/island_wip/tree.obj").c_str(), 
                            ResPath("other/island_wip/").c_str(), 
                            treeEntRef);
    Entity::AddRenderable(treeEntRef, treeModel);
    gs.entities.push_back(treeEntRef);
    EntityData& treeEnt = Entity::GetEntity(treeEntRef);
    PhysicsAddModel(treeModel, treeEnt.transform);
    
    Transform bushTf = Transform({-10,7.5,3}, glm::vec3(0.75));
    EntityRef bushEntRef = Entity::CreateEntity("bush", bushTf);
    Model bushModel = Model(lightingShader, 
                            ResPath("other/island_wip/bush.obj").c_str(), 
                            ResPath("other/island_wip/").c_str(),
                            bushEntRef);
    Entity::AddRenderable(bushEntRef, bushModel);
    gs.entities.push_back(bushEntRef);
    PhysicsAddModel(bushModel, bushTf);

    // pond
    init_main_pond(gs);

    // grass
    init_grass(gs);

    // waterfall
    init_waterfall(gs);
#endif
    
#ifdef SPONZA_SCENE
    Transform sponzaTf = Transform({0,0,0}, glm::vec3(0.1));
    EntityRef sponzaEnt = Entity::CreateEntity("sponza", sponzaTf);
    Model sponza = Model(lightingShader, ResPath("Sponza/sponza.obj").c_str(), ResPath("Sponza/").c_str(), sponzaEnt);
    Entity::AddRenderable(sponzaEnt, sponza);
    gs.entities.push_back(sponzaEnt);
    PhysicsAddModel(sponza, sponzaTf);
#endif

    // Init lights
    glm::vec3 sunPos = glm::vec3(7, 13, -22);
    glm::vec3 sunTarget = glm::vec3(0, 0, 0);
    glm::vec3 sunDir = glm::normalize(sunTarget - sunPos);
    CreateDirectionalLight(sunDir, sunPos, glm::vec4(1), 1.0);
    CreatePointLight(glm::vec3(0,0.2,0), glm::vec4(1));

    { PROFILE_SCOPE("Skybox Init");
        gs.skybox = Skybox({}, TextureProperties::RGB_LINEAR());
    }

}

// TODO: now that renderer is seperate, make render functions not return Framebuffer
Framebuffer testbed_render(const Arena* const gameMem) {
    PROFILE_FUNCTION();
    // gamestate is the first thing allocated always
    const GameState& gs =  *(GameState*)gameMem->backing_mem;

    // update Waves
    #ifdef ISLAND_SCENE
    if (EntityData& waveEntity = Entity::GetEntity("PondEntity")) 
    {
        Model& waveModel = waveEntity.model;
        if (waveEntity && waveModel.isValid()) 
        {
            waveModel.setUniform("numActiveWaves", gs.numActiveWaves);
            waveModel.setUniform("numActiveWaves", gs.numActiveWaves);
            for (u32 i = 0; i < NUM_WAVES; i++) 
            {
                const Wave& wave = gs.waves[i];
                waveModel.setUniform(TextFormat("waves[%i].waveSpeed", i), wave.waveSpeed);
                waveModel.setUniform(TextFormat("waves[%i].wavelength", i), wave.wavelength);
                waveModel.setUniform(TextFormat("waves[%i].steepness", i), wave.steepness);
                waveModel.setUniform(TextFormat("waves[%i].direction", i), wave.direction);
            }
        }
    }

    // grass
    EntityData& grass = Entity::GetEntity("grass");
    if (grass && enableGrassRender) 
    {
        grass.model.setUniform("_WindStrength", gs.windStrength);
        grass.model.setUniform("_WindFrequency", gs.windFrequency);
        grass.model.setUniform("_WindUVScale", gs.windUVScale);
        grass.model.setUniform("_CurveIntensity", gs.grassCurveIntensity);
        grass.model.TryAddSampler(gs.windTexture, "windTexture");
    }
    #endif
    
    { PROFILE_SCOPE("EntityDrawing");
        for (EntityRef ref : gs.entities) 
        {
            EntityData& ent = Entity::GetEntity(ref);
            Renderer::PushEntity(ref);
        }
    }

    return {};
}

void testbed_tick(Arena* gameMem, f32 deltaTime) {
    PROFILE_FUNCTION();
    GameState& gs = *(GameState*)gameMem->backing_mem;
    testbed_camera_tick();
    // have main directional light orbit
    LightingSystem* lightsSystem = GetEngineCtx().lightsSubsystem;
    LightDirectional& mainLight = lightsSystem->lights.sunlight;
    //testbed_orbit_light(mainLight, gs.sunOrbitRadius, gs.sunSpeedMultiplier, gs.sunTarget);
    drawImGuiDebug(gs);
}

void testbed_terminate(Arena* gameMem) {
    GameState& gs = *(GameState*)gameMem->backing_mem;
    //ImGuiTerminate();
    gs.Terminate();
}

AppRunCallbacks GetTestbedAppRunCallbacks()
{
    AppRunCallbacks out;
    out.initFunc = testbed_init;
    out.tickFunc = testbed_tick;
    out.renderFunc = testbed_render;
    out.terminateFunc = testbed_terminate;
    return out;
}

void testbed_standalone_entrypoint(int argc, char *argv[])
{
    const char* resourceDirectory = "../res/";
    if (argc < 2)
    {
        LOG_WARN("no resource directory passed. Using default ./res/");
    }
    else
    {
        resourceDirectory = argv[1];
    }
    LOG_INFO("hi no extras");
    InitEngine(
        resourceDirectory,
        "Testbed",
        1280, 720,
        16, 9,
        true,
        GetTestbedAppRunCallbacks(), 
        MEGABYTES_BYTES(10)
    ); 
}
