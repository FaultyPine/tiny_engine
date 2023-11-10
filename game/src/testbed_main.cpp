//#include "pch.h"
#include "testbed_main.h"

#include "tiny_imgui.h"

// =============== testbed_main.h ========================
#include "model.h"
#include "tiny_types.h"
#include "tiny_engine.h"
#include "render/sprite.h"
#include "render/framebuffer.h"
#include "render/skybox.h"
#include "particles/particles.h"
#include "tiny_alloc.h"

struct WorldEntity {
    Transform transform = {};
    Model model = {};
    u32 hash = 0;
    const char* name = nullptr;
    bool isVisible = true;
    WorldEntity(){}
    WorldEntity(const Transform& tf, const Model& mod, const char* name = "") {
        transform = tf;
        model = mod;
        if (strlen(name) < 1) {
            hash = std::hash<f64>{}(GetTime());
        }
        else {
            hash = std::hash<std::string>{}(std::string(name));
        }
        this->name = name;
    }
    void Delete() {
        model.Delete();
    }
    void Draw(const Transform &tf, const std::vector<LightPoint> &lights = {}, LightDirectional sun = {}) {
        if (isVisible) {
            model.Draw(tf, lights, sun);
        }
    }
    bool isValid() { return hash != 0; }
};

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
    return std::hash<std::string>{}(str);
}

struct GameState {
    Arena* gameMem;

    // TODO: use hashmap w/int IDs
    std::vector<WorldEntity> entities = {};
    std::vector<LightPoint> lights = {};
    LightDirectional sunlight = {};

    // Main pond
    #define NUM_WAVES 8
    Wave waves[NUM_WAVES];
    s32 numActiveWaves = 0;
    Shader pondPrepassShader;
    Texture waterTexture;
    // Waterfall particles
    ParticleSystem waterfallParticles = {};
    
    // grass
    WorldEntity grass = {};
    Shader grassPrepassShader;
    std::vector<glm::mat4> grassTransforms = {};
    BoundingBox grassSpawnExclusion = {};
    Texture windTexture = {};
    f32 windStrength = 0;
    f32 windFrequency = 0;
    f32 windUVScale = 0;
    f32 grassCurveIntensity = 0;

    Sprite depthAndNormsSprite;
    Shader depthAndNormsShader;
    Framebuffer depthAndNorms;

    // postprocessing
    Framebuffer postprocessingFB;

    Skybox skybox = {};

    f32 sunOrbitRadius = 100.0f;

    inline static GameState* gs = nullptr;
    // sets the static gamestate to a copy of the passed in gamestate
    static void Initialize(Arena* gameMem)
    {
        GameState* newGamestate = (GameState*)arena_alloc(gameMem, sizeof(GameState));
        *newGamestate = {}; // default init
        newGamestate->gameMem = gameMem;
        gs = newGamestate;
    }
    static GameState& get() 
    {
        return *gs; 
    }
    void Terminate() {
        for (auto& ent : entities) {
            ent.Delete();
        }
    }
    // TODO: return shared_ptr?
    // TODO: use hashmap
    WorldEntity* GetEntity(const char* name) {
        u32 hash = GetHash(std::string(name));
        for (auto& ent : entities) {
            if (ent.hash == hash) {
                return &ent;
            }
        }
        return nullptr;
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
#include "render/shadows.h"


void testbed_inputpoll() {
    Camera& cam = Camera::GetMainCamera();
    f32 cameraSpeed = cam.speed * GetDeltaTime();
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

static f32 _DepthThreshold = 1.002f;
static f32 _DepthThickness = 1.001f;
static f32 _DepthStrength = 1.0f;
static f32 _ColorThreshold = 1.5f;
static f32 _ColorThickness = 1.001f;
static f32 _ColorStrength = 1.5f;
static f32 _NormalThreshold = 2.7f;
static f32 _NormalThickness = 1.001f;
static f32 _NormalStrength = 1.0f;
static bool enableGrassRender = true;

void drawImGuiDebug() {
    GameState& gs = GameState::get();
    //ImGuiBeginFrame();
    
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    for (WorldEntity& ent : gs.entities)
    {
        const char* entityLabel = TextFormat("Position [%s]", ent.name);
        ImGui::DragFloat3(entityLabel, &ent.transform.position[0]);
    }

    ImGui::DragFloat("Sun Orbit radius", &gs.sunOrbitRadius);
    ImGui::Checkbox("Enable grass render", &enableGrassRender);
    ImGui::DragFloat("_DepthThreshold", &_DepthThreshold, 0.0001f);
    ImGui::DragFloat("_DepthThickness", &_DepthThickness, 0.001f);
    ImGui::DragFloat("_DepthStrength", &_DepthStrength, 0.001f);
    ImGui::DragFloat("_ColorThreshold", &_ColorThreshold, 0.001f);
    ImGui::DragFloat("_ColorThickness", &_ColorThickness, 0.001f);
    ImGui::DragFloat("_ColorStrength", &_ColorStrength, 0.001f);
    ImGui::DragFloat("_NormalThreshold", &_NormalThreshold, 0.001f);
    ImGui::DragFloat("_NormalThickness", &_NormalThickness, 0.001f);
    ImGui::DragFloat("_NormalStrength", &_NormalStrength, 0.001f);



    ImGui::DragFloat("wind str", &gs.windStrength, 0.01f);
    ImGui::DragFloat("wind freq", &gs.windFrequency, 0.01f);
    ImGui::DragFloat("wind uvscale", &gs.windUVScale, 0.01f);
    ImGui::DragFloat("grass curve intensity", &gs.grassCurveIntensity, 0.1f);
    if (ImGui::CollapsingHeader("Grass spawn planes")) {
        ImGui::DragFloat3("Grass ex min", &gs.grassSpawnExclusion.min[0], 0.01f);
        ImGui::DragFloat3("Grass ex max", &gs.grassSpawnExclusion.max[0], 0.01f);
    }
    if (WorldEntity* waveEntity = gs.GetEntity("PondEntity")) {
        if (ImGui::CollapsingHeader("Main Pond") && waveEntity->isValid()) {
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

    for (LightPoint& light : gs.lights)
    {
        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::ColorEdit4("Light Color", &light.color[0]);
            ImGui::DragFloat3("Light pos", &light.position[0], 0.1f);
            ImGui::DragFloat("Light intensity", &light.intensity, 0.01f);
            ImGui::DragFloat3("Attenuation params", &light.constant, 0.01f);
            ImGui::Checkbox("Enabled", &light.enabled);
        }
    }
    LightDirectional& meshLight = gs.sunlight;
    if (ImGui::CollapsingHeader("Sunlight"))
    {
        ImGui::ColorEdit4("sunlight Color", &meshLight.color[0]);
        ImGui::DragFloat3("sunlight direction", &meshLight.direction[0], 0.1f);
        ImGui::DragFloat3("sunlight position", &meshLight.position[0], 0.1f);
        ImGui::DragFloat("sunlight intensity", &meshLight.intensity, 0.1f);
        ImGui::Checkbox("sunlight Enabled", &meshLight.enabled);
    }

    #if 0
    static f32 ambientLightIntensity = 0.15f;
    ImGui::DragFloat("Ambient light intensity", &ambientLightIntensity, 0.01f);
    for (auto& ent : gs.entities) {
        ent.model.cachedShader.use();
        ent.model.cachedShader.setUniform("ambientLightIntensity", ambientLightIntensity);
    }
    #endif

    //ImGuiEndFrame();
}

void ShadowMapPrePass() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();
    const ShadowMap& sunShadows = gs.sunlight.shadowMap;
    sunShadows.BeginRender();
    const LightDirectional& sunlight = gs.sunlight;
    for (WorldEntity& ent : gs.entities) {
        //sunShadows.ReceiveShadows(ent.model.cachedShader, sunlight);
        sunShadows.RenderShadowCaster(sunlight, ent.model, ent.transform);
    }
    //sunShadows.ReceiveShadows(gs.grass.model.cachedShader, sunlight); // grass only receives shadows, doesn't cast
    sunShadows.EndRender();
}

void DepthAndNormsPrePass() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();

    gs.depthAndNorms.Bind();
    ClearGLBuffers();
    glm::mat4 viewMat = Camera::GetMainCamera().GetViewMatrix();
    glm::mat4 projMat = Camera::GetMainCamera().GetProjectionMatrix();
    // render entities to a texture where the rgb of each pixel is the normals
    // and the alpha channel is the depth
    for (WorldEntity& ent : gs.entities) {
        glm::mat4 modelMat = ent.transform.ToModelMatrix();
        glm::mat4 mvp = projMat * viewMat * modelMat;
        if (ent.hash == GetHash("PondEntity")) {
            // ewwwwwwww insanely cringe special case handling... (same with the grass prepass shader)
            // models with vertex displacement shaders need their own specialized
            // shader with the vert shader being the one that displaces the verts
            // and the frag shader being our prepass one
            // might need to add a flag to each shader if it has vertex displacement
            
            gs.pondPrepassShader.use();
            gs.pondPrepassShader.setUniform("mvp", mvp);
            gs.pondPrepassShader.setUniform("numActiveWaves", gs.numActiveWaves);
            for (u32 i = 0; i < NUM_WAVES; i++) {
                Wave& wave = gs.waves[i];
                gs.pondPrepassShader.setUniform(TextFormat("waves[%i].waveSpeed", i), wave.waveSpeed);
                gs.pondPrepassShader.setUniform(TextFormat("waves[%i].wavelength", i), wave.wavelength);
                gs.pondPrepassShader.setUniform(TextFormat("waves[%i].steepness", i), wave.steepness);
                gs.pondPrepassShader.setUniform(TextFormat("waves[%i].direction", i), wave.direction);
            }
            ent.model.Draw(gs.pondPrepassShader, ent.transform);

            continue;
        }
        gs.depthAndNormsShader.use();
        gs.depthAndNormsShader.setUniform("mvp", mvp);
        // draw model to texture
        ent.model.DrawMinimal(gs.depthAndNormsShader);
    }
    if (enableGrassRender)
    {
        gs.grassPrepassShader.use();
        gs.grassPrepassShader.setUniform("_WindStrength", gs.windStrength);
        gs.grassPrepassShader.setUniform("_WindFrequency", gs.windFrequency);
        gs.grassPrepassShader.setUniform("_WindUVScale", gs.windUVScale);
        gs.grassPrepassShader.TryAddSampler(gs.windTexture.id, "windTexture");
        gs.grassPrepassShader.setUniform("_CurveIntensity", gs.grassCurveIntensity);
        gs.grass.model.DrawInstanced(gs.grassPrepassShader, gs.grassTransforms.size());
    }
}

void drawGameState() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();

    // update Waves
    if (WorldEntity* waveEntity = gs.GetEntity("PondEntity")) {
        Shader& waveShader = waveEntity->model.cachedShader;
        if (waveEntity->isValid() && waveShader.isValid()) {
            waveShader.use();
            waveShader.setUniform("numActiveWaves", gs.numActiveWaves);
            for (u32 i = 0; i < NUM_WAVES; i++) {
                Wave& wave = gs.waves[i];
                waveShader.setUniform(TextFormat("waves[%i].waveSpeed", i), wave.waveSpeed);
                waveShader.setUniform(TextFormat("waves[%i].wavelength", i), wave.wavelength);
                waveShader.setUniform(TextFormat("waves[%i].steepness", i), wave.steepness);
                waveShader.setUniform(TextFormat("waves[%i].direction", i), wave.direction);
            }
        }
    }

    // grass
    if (gs.grass.isValid() && enableGrassRender) {
        PROFILE_SCOPE("GrassInstancing");
        gs.grass.model.cachedShader.use();
        gs.grass.model.cachedShader.setUniform("_WindStrength", gs.windStrength);
        gs.grass.model.cachedShader.setUniform("_WindFrequency", gs.windFrequency);
        gs.grass.model.cachedShader.setUniform("_WindUVScale", gs.windUVScale);
        gs.grass.model.cachedShader.setUniform("_CurveIntensity", gs.grassCurveIntensity);
        gs.grass.model.cachedShader.TryAddSampler(gs.windTexture.id, "windTexture");
        gs.grass.model.DrawInstanced(gs.grassTransforms.size(), gs.lights, gs.sunlight);
    }
    
    { PROFILE_SCOPE("EntityDrawing");
        for (const auto& ent : gs.entities) {
            ent.model.Draw(ent.transform, gs.lights, gs.sunlight);
        }
    }

    for (LightPoint& pointLight : gs.lights)
    {
        pointLight.Visualize();
    }

    { PROFILE_SCOPE("Skybox draw");
        gs.skybox.Draw(gs.sunlight);
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

void PopulateGrassTransformsFromSpawnPlane(const BoundingBox& spawnExclusion, const std::vector<Vertex>& planeVerts, std::vector<glm::mat4>& grassTransforms, u32 numGrassInstancesToSpawn) {
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
        Transform grassTransform = Transform(glm::vec3(randomPointBetweenVerts.x, grassSpawnHeight, randomPointBetweenVerts.y), glm::vec3(0.5), randRotation);
        grassTransforms.emplace_back(grassTransform.ToModelMatrix());
    }
}

void init_grass(GameState& gs) {
    PROFILE_FUNCTION();
    // area where grass CANNOT spawn
    gs.grassSpawnExclusion = BoundingBox({-5.39, 8, -0.43}, 
                                         {6.67, 8, 6.9});
    //  init grass transforms
    WorldEntity* islandModel = gs.GetEntity("island");
    if (islandModel) {
        Mesh* grassSpawnMesh = islandModel->model.GetMesh("GrassSpawnPlane_Mesh");
        if (grassSpawnMesh) {
            grassSpawnMesh->isVisible = false;
            PopulateGrassTransformsFromSpawnPlane(gs.grassSpawnExclusion, grassSpawnMesh->vertices, gs.grassTransforms, 100000);
        }
    }

    Shader grassShader = Shader(ResPath("shaders/grass.vert").c_str(), ResPath("shaders/grass.frag").c_str());
    gs.grassPrepassShader = Shader(ResPath("shaders/grass.vert").c_str(), ResPath("shaders/prepass.frag").c_str());
    Model grassModel = Model(grassShader, ResPath("other/island_wip/grass_blade.obj").c_str(), ResPath("other/island_wip/").c_str());
    grassModel.EnableInstancing(gs.grassTransforms.data(), sizeof(glm::mat4), gs.grassTransforms.size());
    Transform grassTf = Transform({0,0,0}, {1,1,1});
    gs.grass = WorldEntity(grassTf, grassModel, "grass");
    gs.windTexture = LoadTexture(ResPath("other/distortion.png"));
    gs.windStrength = 0.15;
    gs.windFrequency = 1.1;
    gs.windUVScale = 0.15;
    gs.grassCurveIntensity = 1.0;
}

void init_main_pond(GameState& gs) {
    Shader waterShader = Shader(ResPath("shaders/water.vert").c_str(), ResPath("shaders/water.frag").c_str());
    gs.pondPrepassShader = Shader(ResPath("shaders/water.vert").c_str(), ResPath("shaders/prepass.frag").c_str());
    gs.waterTexture = LoadTexture(ResPath("other/water.png"));
    waterShader.TryAddSampler(gs.waterTexture.id, "waterTexture");
    Model waterPlane = Model(waterShader, {Shapes3D::GenPlaneMesh(30)});
    Transform waterPlaneTf = Transform({0.35, 3.64, 1.1}, {3.68, 1.0, 3.44});
    WorldEntity waterPlaneEnt = WorldEntity(waterPlaneTf, waterPlane, "PondEntity");
    gs.entities.push_back(waterPlaneEnt);
    gs.waves[0] = Wave(0.2, 8.7, 0.05, glm::vec2(1,1));
    gs.waves[1] = Wave(0.5, 2.0, 0.09, glm::vec2(0,1));
    gs.waves[2] = Wave(0.8, 1.0, 0.1, glm::vec2(1,0.4));
    gs.numActiveWaves = 3;
}

void init_waterfall(GameState& gs) {
    Shader waterfallShader = Shader(ResPath("shaders/waterfall.vert"), ResPath("shaders/waterfall.frag"));
    Texture waterfallTex = LoadTexture(ResPath("noise.jpg"));
    waterfallShader.TryAddSampler(waterfallTex.id, "waterfallTex");

    Model waterfallModel = Model(waterfallShader, ResPath("other/island_wip/waterfall.obj").c_str(), ResPath("other/island_wip/").c_str());
    Transform waterfallTf = Transform({0, 0, 0}, {1.0, 1.0, 1.0});
    WorldEntity waterfallEnt = WorldEntity(waterfallTf, waterfallModel, "Waterfall");
    gs.entities.push_back(waterfallEnt);
    /*
    Shader waterfallParticlesShader = Shader(ResPath("shaders/default_3d.vert"), ResPath("shaders/default_3d.frag"));
    Model waterfallParticleModel = Model(waterfallParticlesShader, {Shapes3D::GenCubeMesh()});
    gs.waterfallParticles = ParticleSystem(waterfallParticleModel, 10, true);
    gs.waterfallParticles
                        .AddBehavior(new ParticleEmitTickInterval(35))
                        //.AddBehavior(new ParticleEmitBurst(20))
                        //.AddBehavior(new ParticlesSpreadOut())
                        .AddBehavior(new ParticleSetVelocity(glm::vec3(0, 0.1, 0)))
                        .AddBehavior(new ParticleSetSize(glm::vec3(1)))
                        .AddBehavior(new ParticleDecay(0.01f));
                        //.AddBehavior(new ParticleAlphaDecay(0.005));
    */
}





void testbed_init(Arena* gameMem) {
    PROFILE_FUNCTION();
    
    {
        GameState::Initialize(gameMem);
    }

    GameState& gs = GameState::get();
    Camera::GetMainCamera().cameraPos.y = 10;
    Shader lightingShader = Shader(ResPath("shaders/basic_lighting.vert"), ResPath("shaders/basic_lighting.frag"));

    //Model testModel = Model(lightingShader, UseResPath("other/floating_island/island.obj").c_str(), UseResPath("other/floating_island/").c_str());
    Model testModel = Model(lightingShader, ResPath("other/island_wip/island.obj").c_str(), ResPath("other/island_wip/").c_str());
    //Model testModel = Model(lightingShader, UseResPath("other/HumanMesh.obj").c_str(), UseResPath("other/").c_str());
    //Model testModel = Model(lightingShader, UseResPath("other/cartoon_land/cartoon_land.obj").c_str(), UseResPath("other/cartoon_land/").c_str());
    gs.entities.emplace_back(WorldEntity(Transform({0,0,0}), testModel, "island"));

    Model treeModel = Model(lightingShader, ResPath("other/island_wip/tree.obj").c_str(), ResPath("other/island_wip/").c_str());
    gs.entities.emplace_back(WorldEntity(Transform({10,7.5,3}, glm::vec3(0.7)), treeModel, "tree"));
    
    Model bushModel = Model(lightingShader, ResPath("other/island_wip/bush.obj").c_str(), ResPath("other/island_wip/").c_str());
    gs.entities.emplace_back(WorldEntity(Transform({-10,7.5,3}, glm::vec3(0.75)), bushModel, "bush"));
    
    //Model sponza = Model(lightingShader, ResPath("other/crytek-sponza/sponza.obj").c_str(), ResPath("other/crytek-sponza/").c_str());
    //gs.entities.emplace_back(WorldEntity(Transform({0,0,0}, glm::vec3(0.06)), sponza, "sponza"));

    // pond
    init_main_pond(gs);

    // grass
    init_grass(gs);

    // waterfall
    init_waterfall(gs);

    // Init lights
    glm::vec3 sunPos = glm::vec3(7, 100, -22);
    glm::vec3 sunTarget = glm::vec3(0, 0, 0);
    glm::vec3 sunDir = glm::normalize(sunTarget - sunPos);
    LightDirectional sun = CreateDirectionalLight(sunDir, sunPos, glm::vec4(1));
    gs.sunlight = sun;
    LightPoint meshPointLight = CreatePointLight(glm::vec3(0), glm::vec4(1));
    gs.lights.push_back(meshPointLight);

    { PROFILE_SCOPE("Skybox Init");
    /*
    gs.skybox = Skybox({
        ResPath("skybox/right.jpg").c_str(),
        ResPath("skybox/left.jpg").c_str(),
        ResPath("skybox/top.jpg").c_str(),
        ResPath("skybox/bottom.jpg").c_str(),
        ResPath("skybox/front.jpg").c_str(),
        ResPath("skybox/back.jpg").c_str()}, TextureProperties::RGB_LINEAR());
    */
    gs.skybox = Skybox({}, TextureProperties::RGB_LINEAR());
    }

    // init main game framebuffer
    if (!gs.depthAndNorms.isValid()) {
        gs.depthAndNormsShader = Shader(ResPath("shaders/prepass.vert"), ResPath("shaders/prepass.frag"));
        gs.depthAndNorms = CreateDepthAndNormalsFB((f32)Camera::GetScreenWidth(), (f32)Camera::GetScreenHeight());
        gs.depthAndNormsSprite = Sprite(gs.depthAndNorms.GetTexture());
    }
    if (!gs.postprocessingFB.isValid()) {
        gs.postprocessingFB = Framebuffer(Camera::GetScreenWidth(), Camera::GetScreenHeight(), Framebuffer::FramebufferAttachmentType::COLOR);
        Shader postprocessingShader = Shader(ResPath("shaders/screen_texture.vert"), ResPath("shaders/screen_texture.frag"));
        postprocessingShader.TryAddSampler(gs.depthAndNorms.GetTexture().id, "depthNormals");
    }
}

Framebuffer testbed_render(const Arena* const gameMem) {
    GameState& gs = GameState::get();
    #if 0
    SetWireframeDrawing(true);
    #endif

    ShadowMapPrePass();
    DepthAndNormsPrePass();

    // draw game to postprocessing framebuffer
    gs.postprocessingFB.Bind();

    ClearGLBuffers();
    drawGameState();

#if 1
    {
        // render shadowmap tex to screen
        glm::vec2 scrn = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
        gs.sunlight.shadowMap.fb.DrawToFramebuffer(gs.postprocessingFB, Transform2D(glm::vec2(0), scrn/3.0f));
    }
#endif
#if 1
    {    
        // render normals+depth tex to screen
        glm::vec2 scrn = {Camera::GetScreenWidth(), Camera::GetScreenHeight()};
        gs.depthAndNorms.DrawToFramebuffer(gs.postprocessingFB, Transform2D(glm::vec2(0, scrn.y / 2.0f), scrn/3.0f));
    }
#endif
    BoundingBox box = gs.entities[0].model.cachedBoundingBox;
    Shapes3D::DrawWireCube(box);
    

#ifdef ENABLE_IMGUI
    drawImGuiDebug();
#endif
    Framebuffer::BindDefaultFrameBuffer();
    // red is x, green is y, blue is z
    // should put this on the screen in the corner permanently
    Shapes3D::DrawLine(glm::vec3(0), {1,0,0}, {1,0,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,1,0}, {0,1,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,0,1}, {0,0,1,1});
    return gs.postprocessingFB;
}

void testbed_tick(Arena* gameMem) {
    GameState& gs = GameState::get();
    testbed_inputpoll();
    // have main directional light orbit
    LightDirectional& mainLight = gs.sunlight;
    testbed_orbit_light(mainLight, gs.sunOrbitRadius, 0.2, glm::vec3(0, 10, 0));
    //gs.waterfallParticles.Tick({0,15,0});
}

void testbed_terminate(Arena* gameMem) {
    //ImGuiTerminate();
    GameState::get().Terminate();
}

void GetTestbedAppRunCallbacks(AppRunCallbacks* out)
{
    out->initFunc = testbed_init;
    out->tickFunc = testbed_tick;
    out->renderFunc = testbed_render;
    out->terminateFunc = testbed_terminate;
}

void testbed_standalone_entrypoint(int argc, char *argv[])
{
    EngineState engineInitState;
    engineInitState.appName = "Testbed";
    engineInitState.windowWidth = 1920;
    engineInitState.windowHeight = 1080;
    engineInitState.aspectRatioW = 16;
    engineInitState.aspectRatioH = 9;
    engineInitState.false2DTrue3D = true;
    GetTestbedAppRunCallbacks(&engineInitState.appCallbacks);
    InitEngine(engineInitState, MEGABYTES_BYTES(20), argc, argv); 
}
