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
    if (Keyboard::isKeyDown(GLFW_KEY_W)) {
        cam.cameraPos += cameraSpeed * cam.cameraFront;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_S)) {
        cam.cameraPos -= cameraSpeed * cam.cameraFront;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_A)) {
        glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
        cam.cameraPos -= cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_D)) {
        glm::vec3 cameraRight = glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp));
        cam.cameraPos += cameraRight * cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_SPACE)) {
        cam.cameraPos += cam.cameraUp * cameraSpeed;
    }
    if (Keyboard::isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        cam.cameraPos -= cam.cameraUp * cameraSpeed;
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
    
    Light& meshLight = gs.lights[0];
    ImGuiBeginFrame();
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    WorldEntity& box = *gs.GetEntity("cubeModel");
        ImGui::DragFloat3("Box pos", &box.transform.position[0]);
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

    for (Light& light : gs.lights) {
        light.Visualize();
        if (light.type == LIGHT_DIRECTIONAL)
            Shapes3D::DrawLine(light.position, light.position + glm::normalize(light.target-light.position), {1.0, 1.0, 1.0, 1.0});
    }
}

void testbed_init() {
    InitImGui();
    GameState& gs = GameState::get();
    Shader shader = Shader(UseResPath("shaders/lighting.vs").c_str(), UseResPath("shaders/lighting.fs").c_str());
    Light meshLight = CreateLight(LIGHT_DIRECTIONAL, glm::vec3(5, 10, 5), glm::vec3(0), glm::vec4(1));
    //Light meshPointLight = CreateLight(LIGHT_POINT, glm::vec3(2, 7, 8), glm::vec3(0), glm::vec4(1));
    
    Model testModel;
    testModel = Model(shader, UseResPath("other/floating_island/island.obj").c_str(), UseResPath("other/floating_island/").c_str());
    //testModel = Model(shader, UseResPath("other/HumanMesh.obj").c_str(), UseResPath("other/").c_str());
    //testModel = Model(shader, UseResPath("other/cartoon_land/cartoon_land.obj").c_str(), UseResPath("other/cartoon_land/").c_str());
    
    gs.entities.emplace_back(WorldEntity(Transform({0,0,0}), testModel, "testModel"));
    //Model boxModel = Model(shader, UseResPath("other/blender_cube.obj").c_str(), UseResPath("other/").c_str());
    //gs.entities.emplace_back(WorldEntity(Transform({4,6,8}), boxModel, "box"));
    
    Mesh cube = Shapes3D::GenCubeMesh();
    Model cubeModel = Model(shader, {cube});
    gs.entities.push_back(WorldEntity(Transform({0,5,0}), cubeModel, "cubeModel"));

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

    // render depth tex to screen
    glm::vec2 scrn = {Camera::GetMainCamera().GetScreenWidth(), Camera::GetMainCamera().GetScreenHeight()};
    //depthSprite.DrawSprite(Camera::GetMainCamera(), {0,0}, scrn/2.8f, 0, {0,0,1}, {1,1,1,1}, false, true);
}

void testbed_tick() {
    GameState& gs = GameState::get();
    testbed_inputpoll();
    //testbed_orbit_cam(27, 17, {0, 10, 0});
    // have main directional light orbit
    Light& mainLight = gs.lights[0];
    testbed_orbit_light(mainLight, 35, 25, 0.3);

    DepthPrePass();

    // render normal scene
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