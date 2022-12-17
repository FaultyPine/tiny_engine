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
#include "tiny_engine/framebuffer.h"
#include "tiny_engine/sprite.h"

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

void testbed_orbit(f32 orbitRadius, f32 cameraOrbitHeight, glm::vec3 lookAtPos) {
    Camera& cam = Camera::GetMainCamera();
    f32 time = (f32)GetTime();
    f32 x = sinf(time) * orbitRadius;
    f32 z = cosf(time) * orbitRadius;
    cam.cameraPos = glm::vec3(x, cameraOrbitHeight, z);
    cam.LookAt(lookAtPos);
}


void drawImGuiDebug() {
    GameState& gs = GameState::get();
    
    Light& meshLight = gs.lights[0];
    ImGuiBeginFrame();
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    WorldEntity& box = gs.entities[1];
        ImGui::DragFloat3("Box pos", &box.transform.position[0]);
    ImGui::ColorEdit4("Light Color", &meshLight.color[0]);
    ImGui::DragFloat3("Light pos", &meshLight.position[0]);
    ImGui::DragFloat3("Light target", &meshLight.target[0]);
    ImGui::SliderInt("1=point 0=directional", (int*)&meshLight.type, 0, 1);
    WorldEntity& testMesh = gs.entities[0];
        ImGui::DragFloat3("Mesh pos", &testMesh.transform.position[0]);
        ImGui::DragFloat3("Mesh scale", &testMesh.transform.scale[0]);
        ImGui::DragFloat("Mesh rotation", &testMesh.transform.rotation);
        ImGui::DragFloat3("Mesh rotation axis", &testMesh.transform.rotationAxis[0]);
    ImGuiEndFrame();
}

void drawGameState() {
    GameState& gs = GameState::get();
    WorldEntity* testModel = gs.GetEntity("testModel");
    testModel->model.Draw(testModel->transform.position, testModel->transform.scale, 
                        testModel->transform.rotation, testModel->transform.rotationAxis, gs.lights);
    WorldEntity* box = gs.GetEntity("box");
    box->model.Draw(box->transform.position, box->transform.scale,
                    box->transform.rotation, box->transform.rotationAxis, gs.lights);

    for (Light& light : gs.lights) {
        light.Visualize();
        if (light.type == LIGHT_DIRECTIONAL)
            Shapes3D::DrawLine(light.position, light.position + glm::normalize(light.target-light.position), {1.0, 1.0, 1.0, 1.0});
    }
}

void testbed_init() {
    InitImGui();
    Shader shader = Shader(UseResPath("shaders/lighting.vs").c_str(), UseResPath("shaders/lighting.fs").c_str());
    Light meshLight = CreateLight(LIGHT_DIRECTIONAL, glm::vec3(5, 10, 5), glm::vec3(0), glm::vec4(1));
    Light meshPointLight = CreateLight(LIGHT_POINT, glm::vec3(2, 7, 8), glm::vec3(0), glm::vec4(1));
    
    Model testModel;
    //testModel = Model(shader, UseResPath("other/floating_island/island.obj").c_str(), UseResPath("other/floating_island/").c_str());
    testModel = Model(shader, UseResPath("other/HumanMesh.obj").c_str(), UseResPath("other/").c_str());
    //testModel = Model(shader, UseResPath("other/cartoon_land/cartoon_land.obj").c_str(), UseResPath("other/cartoon_land/").c_str());
    
    GameState::get().entities.emplace_back(WorldEntity(Transform({0,0,0}), testModel, "testModel"));
    Model boxModel = Model(shader, UseResPath("other/blender_cube.obj").c_str(), UseResPath("other/").c_str());
    GameState::get().entities.emplace_back(WorldEntity(Transform({4,6,8}), boxModel, "box"));

    GameState::get().lights.push_back(meshLight);
    GameState::get().lights.push_back(meshPointLight);
}

void testbed_tick() {
    GameState& gs = GameState::get();
    testbed_inputpoll();
    //testbed_orbit(27, 17, {0, 10, 0});

    static ShadowMap shadowMap;
    static Sprite depthSprite;
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
    depthSprite.DrawSprite(Camera::GetMainCamera(), {0,0}, scrn/2.8f, 0, {0,0,1}, {1,1,1,1}, false, true);

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