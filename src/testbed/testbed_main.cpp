#include "pch.h"
#include "testbed_main.h"

#include "tiny_engine/camera.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/input.h"
#include "tiny_engine/ObjParser.h"
#include "tiny_engine/shader.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/mesh.h"
#include "tiny_engine/texture.h"
#include "tiny_engine/tiny_lights.h"
#include "tiny_engine/math.h"
#include "tiny_engine/external/imgui/tiny_imgui.h"
#include "tiny_engine/model.h"
#include "tiny_engine/shapes.h"
#include "tiny_engine/tiny_profiler.h"
#include "tiny_engine/particles/particle_behaviors.h"

void testbed_inputpoll() {
    Camera& cam = Camera::GetMainCamera();
    f32 cameraSpeed = cam.speed * GetDeltaTime();
    glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
    if (Keyboard::isKeyDown(GLFW_KEY_W)) {
        cam.cameraPos += cameraSpeed * glm::vec3(cam.cameraFront.x, 0.0, cam.cameraFront.z);
    }
    if (Keyboard::isKeyDown(GLFW_KEY_S)) {
        cam.cameraPos -= cameraSpeed * glm::vec3(cam.cameraFront.x, 0.0, cam.cameraFront.z);
    }
    if (Keyboard::isKeyDown(GLFW_KEY_A)) {
        cam.cameraPos -= cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_D)) {
        cam.cameraPos += cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_SPACE)) {
        cam.cameraPos.y += cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        cam.cameraPos.y -= cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_ESCAPE)) {
        CloseGameWindow();
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_TAB)) {
        // when "tabbing" in and out of the game, the cursor position jumps around weirdly
        // so here we save the last cursor pos when we tab out, and re-set it when we tab back in
        static glm::vec2 lastMousePos = glm::vec2(0);
        if (glfwGetInputMode(glob_glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cam.isSwivelable = true;
            glfwSetCursorPos(glob_glfw_window, lastMousePos.x, lastMousePos.y);
        }
        else {
            lastMousePos = {MouseInput::GetMouse().lastX, MouseInput::GetMouse().lastY};
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);  
            cam.isSwivelable = false;
        }
    }


    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        Shader::ReloadShaders();
        GameState& gs = GameState::get();
        
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
void testbed_orbit_light(Light& light, f32 orbitRadius, f32 speedMultiplier) {
    f32 time = (f32)GetTime()*speedMultiplier;
    f32 x = sinf(time) * orbitRadius;
    f32 z = cosf(time) * orbitRadius;
    light.position.x = x;
    light.position.z = z;
}

void drawImGuiDebug() {
    GameState& gs = GameState::get();
    ImGuiBeginFrame();
    
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::DragFloat("wind str", &gs.windStrength, 0.01f);
    ImGui::DragFloat("wind freq", &gs.windFrequency, 0.01f);
    ImGui::DragFloat("wind uvscale", &gs.windUVScale, 1.0f);
    if (ImGui::CollapsingHeader("Grass spawn planes")) {
        ImGui::DragFloat3("Grass ex min", &gs.grassSpawnExclusion.min[0], 0.01f);
        ImGui::DragFloat3("Grass ex max", &gs.grassSpawnExclusion.max[0], 0.01f);
    }
    if (ImGui::CollapsingHeader("Main Pond") && gs.waveEntity.isValid()) {
        if (ImGui::CollapsingHeader("Wave Plane")) {
            //ImGui::DragFloat3("Wave pos", &gs.waveEntity.transform.position[0], 0.01f);
            //ImGui::DragFloat3("Wave scale", &gs.waveEntity.transform.scale[0], 0.01f);
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
    Light& meshLight = gs.lights[0];
    ImGui::ColorEdit4("Light Color", &meshLight.color[0]);
    ImGui::DragFloat3("Light pos", &meshLight.position[0], 0.1f);
    ImGui::DragFloat3("Light target", &meshLight.target[0]);
    ImGui::SliderInt("1=point 0=directional", (int*)&meshLight.type, 0, 1);
    
    #if 0
    static f32 ambientLightIntensity = 0.1f;
    ImGui::DragFloat("Ambient light intensity", &ambientLightIntensity, 0.01f);
    for (auto& ent : gs.entities) {
        ent.model.cachedShader.use();
        ent.model.cachedShader.setUniform("ambientLightIntensity", ambientLightIntensity);
    }
    #endif

    ImGuiEndFrame();
}

void DepthPrePass() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();
    ShadowMap& shadowMap = gs.shadowMap;
    Sprite& depthSprite = gs.depthSprite; 
    if (!shadowMap.isValid()) {
        shadowMap = ShadowMap(2048);
        depthSprite = Sprite(shadowMap.fb.GetTexture());
    }

    shadowMap.BeginRender();
    const Light& sunlight = gs.lights[0];
    for (auto& ent : gs.entities) {
        shadowMap.ReceiveShadows(ent.model.cachedShader, sunlight);
        shadowMap.RenderShadowCaster(sunlight, ent.model, ent.transform);
    }
    shadowMap.ReceiveShadows(gs.grass.model.cachedShader, sunlight); // grass only receives shadows, doesn't cast
    shadowMap.EndRender();

    #if 0
    // render depth tex to screen
    glm::vec2 scrn = {Camera::GetMainCamera().GetScreenWidth(), Camera::GetMainCamera().GetScreenHeight()};
    depthSprite.DrawSprite({0,0}, scrn/3.0f, 0, {0,0,1}, {1,1,1,1}, false, true);
    #endif
}

void drawGameState() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();

    { PROFILE_SCOPE("EntityDrawing");
        for (const auto& ent : gs.entities) {
            ent.model.Draw(ent.transform, gs.lights);
        }
    }

    // Waves
    Shader& waveShader = gs.waveEntity.model.cachedShader;
    if (gs.waveEntity.isValid() && waveShader.isValid()) {
        waveShader.use();
        waveShader.setUniform("numActiveWaves", gs.numActiveWaves);
        for (u32 i = 0; i < NUM_WAVES; i++) {
            Wave& wave = gs.waves[i];
            waveShader.setUniform(TextFormat("waves[%i].waveSpeed", i), wave.waveSpeed);
            waveShader.setUniform(TextFormat("waves[%i].wavelength", i), wave.wavelength);
            waveShader.setUniform(TextFormat("waves[%i].steepness", i), wave.steepness);
            waveShader.setUniform(TextFormat("waves[%i].direction", i), wave.direction);
        }
        waveShader.ActivateSamplers();
        gs.waveEntity.model.Draw(gs.waveEntity.transform);
    }

    // grass
    if (gs.grass.isValid()) {
        PROFILE_SCOPE("GrassInstancing");
        gs.grass.model.cachedShader.use();
        gs.grass.model.cachedShader.setUniform("_WindStrength", gs.windStrength);
        gs.grass.model.cachedShader.setUniform("_WindFrequency", gs.windFrequency);
        gs.grass.model.cachedShader.setUniform("_WindUVScale", gs.windUVScale);
        gs.grass.model.cachedShader.TryAddSampler(gs.windTexture.id, "windTexture");
        gs.grass.model.DrawInstanced(gs.grassTransforms.size());
    }
    // waterfall
    WorldEntity* island = gs.GetEntity("island");
    if (island) {
        Mesh* waterfallMesh = island->model.GetMesh("WaterfallPlane_Plane.001");
        if (waterfallMesh) {
            waterfallMesh->isVisible = true;
            waterfallMesh->Draw(gs.waterfallShader, Transform({0,0,0}));
            waterfallMesh->isVisible = false;
        }
    }
    //gs.waterfallParticles.Draw();

    /*for (Light& light : gs.lights) {
        light.Visualize();
        if (light.type == LIGHT_DIRECTIONAL)
            Shapes3D::DrawLine(light.position, light.position + light.Direction(), {1.0, 1.0, 1.0, 1.0});
    }*/
    
    #if 1
    { PROFILE_SCOPE("Skybox draw");
        if (gs.skybox.skyboxShader.isValid()) {
            gs.skybox.skyboxShader.use();
            gs.skybox.skyboxShader.setUniform("sunDirection", gs.lights[0].Direction());
            gs.skybox.Draw();
        }
    }
    #endif
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
        Transform grassTransform = Transform(glm::vec3(randomPointBetweenVerts.x, grassSpawnHeight, randomPointBetweenVerts.y), glm::vec3(0.5));
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
            PopulateGrassTransformsFromSpawnPlane(gs.grassSpawnExclusion, grassSpawnMesh->vertices, gs.grassTransforms, 7000);
        }
    }

    Shader grassShader = Shader(ResPath("shaders/grass.vs").c_str(), ResPath("shaders/grass.fs").c_str());
    Model grassModel = Model(grassShader, ResPath("other/island_wip/grass_blade.obj").c_str(), ResPath("other/island_wip/").c_str());
    grassModel.EnableInstancing(gs.grassTransforms.data(), sizeof(glm::mat4), gs.grassTransforms.size());
    Transform grassTf = Transform({0,0,0}, {1,1,1});
    gs.grass = WorldEntity(grassTf, grassModel, "grass");
    gs.windTexture = LoadTexture(ResPath("other/distortion.png"));
    gs.windStrength = 0.09;
    gs.windFrequency = 9.55;
    gs.windUVScale = 500.0;
}

void init_main_pond(GameState& gs) {
    Shader waterShader = Shader(ResPath("shaders/water.vs").c_str(), ResPath("shaders/water.fs").c_str());
    gs.waterTexture = LoadTexture(ResPath("other/water.png"));
    waterShader.TryAddSampler(gs.waterTexture.id, "waterTexture");
    Model waterPlane = Model(waterShader, {Shapes3D::GenPlaneMesh(30)});
    Transform waterPlaneTf = Transform({0.35, 3.64, 1.1}, {3.68, 1.0, 3.44});
    WorldEntity waterPlaneEnt = WorldEntity(waterPlaneTf, waterPlane, "waterPlane");
    gs.waveEntity = waterPlaneEnt;
    gs.waves[0] = Wave(0.2, 8.7, 0.05, glm::vec2(1,1));
    gs.waves[1] = Wave(0.5, 2.0, 0.09, glm::vec2(0,1));
    gs.waves[2] = Wave(0.8, 1.0, 0.1, glm::vec2(1,0.4));
    gs.numActiveWaves = 3;
}

void init_waterfall(GameState& gs, Mesh& waterfallMesh) {
    // disable normal drawing of this mesh so we can do it
    // manually with our custom waterfall shader
    waterfallMesh.isVisible = false;
    gs.waterfallShader = Shader(ResPath("shaders/waterfall.vs"), ResPath("shaders/waterfall.fs"));
    Texture waterfallTex = LoadTexture(ResPath("noise.jpg"));
    gs.waterfallShader.TryAddSampler(waterfallTex.id, "waterfallTex");

    /*
    Shader waterfallParticlesShader = Shader(ResPath("shaders/default_3d.vs"), ResPath("shaders/default_3d.fs"));
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

void testbed_init() {
    PROFILE_FUNCTION();
    
    InitImGui();
    GameState& gs = GameState::get();
    Camera::GetMainCamera().cameraPos.y = 10;
    Shader lightingShader = Shader(ResPath("shaders/basic_lighting.vs"), ResPath("shaders/basic_lighting.fs"));

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
    if (Mesh* waterfallMesh = testModel.GetMesh("WaterfallPlane_Plane.001")) {
        init_waterfall(gs, *waterfallMesh);
    }

    // Init lights
    Light meshLight = CreateLight(LIGHT_DIRECTIONAL, glm::vec3(7, 100, -22), glm::vec3(0, 10, 0), glm::vec4(1));
    //Light meshPointLight = CreateLight(LIGHT_POINT, glm::vec3(2, 7, 8), glm::vec3(0), glm::vec4(1));
    gs.lights.push_back(meshLight);
    //gs.lights.push_back(meshPointLight);

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
}

void testbed_gametick(GameState& gs) {
    testbed_inputpoll();
    //testbed_orbit_cam(27, 17, {0, 10, 0});
    // have main directional light orbit
    Light& mainLight = gs.lights[0];
    testbed_orbit_light(mainLight, 200, 0.2);
    //gs.waterfallParticles.Tick({0,15,0});
}
void testbed_render(GameState& gs) {
    #if 0
    SetWireframeDrawing(true);
    #endif

    DepthPrePass();
    if (!gs.postprocessingFB.isValid()) {
        gs.postprocessingFB = Framebuffer(Camera::GetScreenWidth(), Camera::GetScreenHeight(), Framebuffer::FramebufferAttachmentType::COLOR);
        Shader postprocessingShader = Shader(ResPath("shaders/screen_texture.vs"), ResPath("shaders/outline.fs"));
        //gs.shadowMap.ReceiveShadows(postprocessingShader, gs.lights[0]);
        gs.framebufferSprite = Sprite(postprocessingShader, gs.postprocessingFB.GetTexture());
    }
    gs.postprocessingFB.Bind();
    ClearGLBuffers();
    drawGameState();
    gs.postprocessingFB.BindDefaultFrameBuffer();
    gs.framebufferSprite.GetShader().use();
    //gs.framebufferSprite.setShaderUniform("_OutlineScale", outlineScale);
    //gs.framebufferSprite.setShaderUniform("_DepthThreshold", depthThreshold);
    gs.framebufferSprite.DrawSprite(Transform2D({0,0}, {Camera::GetScreenWidth(), Camera::GetScreenHeight()}), glm::vec4(1), false, true);
    
    drawImGuiDebug();

    // red is x, green is y, blue is z
    Shapes3D::DrawLine(glm::vec3(0), {1,0,0}, {1,0,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,1,0}, {0,1,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,0,1}, {0,0,1,1});
}

void testbed_tick() {
    PROFILE_FUNCTION();
    GameState& gs = GameState::get();
    testbed_gametick(gs);
    testbed_render(gs);
}
void testbed_terminate() {
    ImGuiTerminate();
    GameState::get().Terminate();
}