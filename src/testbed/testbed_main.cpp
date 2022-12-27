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

}

void testbed_orbit_cam(f32 orbitRadius, f32 cameraOrbitHeight, glm::vec3 lookAtPos) {
    Camera& cam = Camera::GetMainCamera();
    f32 time = (f32)GetTime();
    f32 x = sinf(time) * orbitRadius;
    f32 z = cosf(time) * orbitRadius;
    cam.cameraPos = glm::vec3(x, cameraOrbitHeight, z);
    cam.LookAt(lookAtPos);
}
void testbed_orbit_light(Light& light, f32 orbitRadius, f32 orbitHeight, f32 speedMultiplier) {
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
    if (gs.waveEntity.isValid()) {
        if (ImGui::CollapsingHeader("Wave Plane")) {
            ImGui::DragFloat3("Wave pos", &gs.waveEntity.transform.position[0], 0.01f);
            ImGui::DragFloat3("Wave scale", &gs.waveEntity.transform.scale[0], 0.01f);
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
    ImGui::DragFloat3("Light pos", &meshLight.position[0]);
    ImGui::DragFloat3("Light target", &meshLight.target[0]);
    ImGui::SliderInt("1=point 0=directional", (int*)&meshLight.type, 0, 1);
    
    ImGuiEndFrame();
}

void drawGameState() {
    GameState& gs = GameState::get();
    for (auto& ent : gs.entities) {
        ent.model.Draw(ent.transform.position, ent.transform.scale, 
                ent.transform.rotation, ent.transform.rotationAxis, gs.lights);
    }

    // Waves
    Shader& waveShader = gs.waveEntity.model.cachedShader;
    if (gs.waveEntity.isValid()) {
        waveShader.use();
        for (u32 i = 0; i < NUM_WAVES; i++) {
            Wave& wave = gs.waves[i];
            waveShader.setUniform(TextFormat("waves[%i].waveSpeed", i), wave.waveSpeed);
            waveShader.setUniform(TextFormat("waves[%i].wavelength", i), wave.wavelength);
            waveShader.setUniform(TextFormat("waves[%i].steepness", i), wave.steepness);
            waveShader.setUniform(TextFormat("waves[%i].direction", i), wave.direction);
        }
        gs.waterTexture.bindUnit(0);
        gs.waveEntity.model.Draw(gs.waveEntity.transform.position, gs.waveEntity.transform.scale, 
            gs.waveEntity.transform.rotation, gs.waveEntity.transform.rotationAxis);
    }

    for (Light& light : gs.lights) {
        light.Visualize();
        if (light.type == LIGHT_DIRECTIONAL)
            Shapes3D::DrawLine(light.position, light.position + glm::normalize(light.target-light.position), {1.0, 1.0, 1.0, 1.0});
    }
}

void testbed_init() {
    InitImGui();
    GameState& gs = GameState::get();
    Shader lightingShader = Shader(UseResPath("shaders/lighting.vs").c_str(), UseResPath("shaders/lighting.fs").c_str());
    Light meshLight = CreateLight(LIGHT_DIRECTIONAL, glm::vec3(5, 10, 5), glm::vec3(0), glm::vec4(1));
    //Light meshPointLight = CreateLight(LIGHT_POINT, glm::vec3(2, 7, 8), glm::vec3(0), glm::vec4(1));
    
    Model testModel;
    //testModel = Model(lightingShader, UseResPath("other/floating_island/island.obj").c_str(), UseResPath("other/floating_island/").c_str());
    testModel = Model(lightingShader, UseResPath("other/island_wip/island.obj").c_str(), UseResPath("other/island_wip/").c_str());
    //testModel = Model(lightingShader, UseResPath("other/HumanMesh.obj").c_str(), UseResPath("other/").c_str());
    //testModel = Model(lightingShader, UseResPath("other/cartoon_land/cartoon_land.obj").c_str(), UseResPath("other/cartoon_land/").c_str());
    gs.entities.emplace_back(WorldEntity(Transform({0,0,0}), testModel, "testModel"));
    
    Shader waterShader = Shader(UseResPath("shaders/water.vs").c_str(), UseResPath("shaders/water.fs").c_str());
    Model waterPlane = Model(waterShader, {Shapes3D::GenPlaneMesh(30)});
    Transform waterPlaneTf = Transform({0.26, -0.5, -0.36}, {4.41, 1.0, 3.84});
    WorldEntity waterPlaneEnt = WorldEntity(waterPlaneTf, waterPlane, "waterPlane");
    gs.waveEntity = waterPlaneEnt;
    gs.waves[0] = Wave(0.2, 8.7, 0.15, glm::vec2(1,1));
    gs.waves[1] = Wave(0.5, 2.0, 0.25, glm::vec2(0,1));
    gs.waves[2] = Wave(0.8, 1.0, 0.60, glm::vec2(1,0.4));
    waterShader.use();
    waterShader.setUniform("numActiveWaves", 3);
    gs.waterTexture = LoadTexture(UseResPath("other/water.png"));
    waterShader.setUniform("waterTexture", 0);
    gs.lights.push_back(meshLight);
    //gs.lights.push_back(meshPointLight);
}

void DepthPrePass() {
    GameState& gs = GameState::get();
    ShadowMap& shadowMap = gs.shadowMap;
    Sprite& depthSprite = gs.depthSprite; 
    if (!shadowMap.isValid()) {
        shadowMap = ShadowMap(1024);
        depthSprite = Sprite(shadowMap.fb.GetTexture());
    }

    shadowMap.BeginRender();
    for (auto& ent : gs.entities) {
        shadowMap.RenderToShadowMap(gs.lights[0], ent.model, ent.transform, 0);
    }
    shadowMap.EndRender();

    #if 0
    // render depth tex to screen
    glm::vec2 scrn = {Camera::GetMainCamera().GetScreenWidth(), Camera::GetMainCamera().GetScreenHeight()};
    depthSprite.DrawSprite(Camera::GetMainCamera(), {0,0}, scrn/3.0f, 0, {0,0,1}, {1,1,1,1}, false, true);
    #endif
}

void testbed_tick() {
    GameState& gs = GameState::get();
    testbed_inputpoll();
    //testbed_orbit_cam(27, 17, {0, 10, 0});
    // have main directional light orbit
    Light& mainLight = gs.lights[0];
    testbed_orbit_light(mainLight, 35, 25, 0.02);

    DepthPrePass();

    // render normal scene
    #if 0
    SetWireframeDrawing(true);
    #endif
    drawGameState();
    drawImGuiDebug();

    Shapes3D::DrawLine(glm::vec3(0), {1,0,0}, {1,0,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,1,0}, {0,1,0,1});
    Shapes3D::DrawLine(glm::vec3(0), {0,0,1}, {0,0,1,1});
}
void testbed_terminate() {
    ImGuiTerminate();
    GameState::get().Terminate();
}