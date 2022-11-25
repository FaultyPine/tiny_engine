#include "testbed_main.h"

#include "tiny_engine/pch.h"
#include "tiny_engine/camera.h"
#include "tiny_engine/tiny_engine.h"
#include "tiny_engine/input.h"
#include "tiny_engine/ObjParser.h"
#include "tiny_engine/shader.h"
#include "tiny_engine/tiny_fs.h"
#include "tiny_engine/mesh.h"
#include "tiny_engine/texture.h"
#include "tiny_engine/tiny_lights.h"
#include "tiny_engine/external/imgui/tiny_imgui.h"


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
        if (glfwGetInputMode(glob_glfw_window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(glob_glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cam.isSwivelable = true;
            cam.LookAt({0,cam.cameraPos.y,0});
        }
        else {
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


std::vector<Mesh> testModel = {};
Mesh testMesh;
glm::vec3 testMeshPos = glm::vec3(0);
f32 testMeshScale = 1.0;
f32 testMeshRotation = 0.0;
glm::vec3 testMeshRotationAxis = glm::vec3(1, 0, 0);
Light meshLight = {0};
f32 shininess = 16.0;

void drawGameState() {
    f32 time = (f32)GetTime();

    // TODO: abstract this multiple mesh setup into a "Model" object
    for (auto& mesh : testModel) {
        Shader& shader = mesh.GetShader();
        shader.use();
        shader.setUniform("viewPos", Camera::GetMainCamera().cameraPos);
        mesh.material.shininess = shininess;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(testMeshRotation), testMeshRotationAxis); 
        model = glm::translate(model, testMeshPos);
        model = glm::scale(model, glm::vec3(testMeshScale));
        shader.setUniform("modelMat", model);
        UpdateLightValues(shader, meshLight);
        mesh.Draw(testMeshPos, testMeshScale, testMeshRotation, testMeshRotationAxis);
    }
    meshLight.Visualize();

    ImGuiBeginFrame();
    ImGui::Text("avg tickrate %.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::ColorEdit4("Light Color", &meshLight.color[0]);
    ImGui::DragFloat3("Light pos", &meshLight.position[0]);
    ImGui::DragFloat3("Light target", &meshLight.target[0]);
    ImGui::DragFloat("Shininess", &shininess);
    ImGui::SliderInt("1=point 0=directional", (int*)&meshLight.type, 0, 1);
    ImGui::DragFloat3("Mesh pos", &testMeshPos[0]);
    ImGui::DragFloat("Mesh scale", &testMeshScale);
    ImGui::DragFloat("Mesh rotation", &testMeshRotation);
    ImGui::DragFloat3("Mesh rotation axis", &testMeshRotationAxis[0]);
    ImGuiEndFrame();
}

void testbed_init() {
    InitImGui();
    if (!testMesh.isValid()) {
        Shader shader = Shader(UseResPath("shaders/lighting.vs").c_str(), UseResPath("shaders/lighting.fs").c_str());
        meshLight = CreateLight(LIGHT_DIRECTIONAL, glm::vec3(5, 10, 5), glm::vec3(0), glm::vec4(1), shader);

        testModel = load_obj(shader, UseResPath("other/floating_island/island.obj").c_str(), UseResPath("other/floating_island/").c_str());
        //testModel = load_obj(shader, UseResPath("other/HumanMesh.obj").c_str(), UseResPath("other/").c_str());
    }

}
void testbed_tick() {
    testbed_inputpoll();
    //testbed_orbit(27, 17, {0, 10, 0});
    drawGameState();
}
void testbed_terminate() {
    ImGuiTerminate();
}