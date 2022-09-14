#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

#include "pch.h"
#include "tiny_fs.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "input.h"
#include "sprite.h"
#include "math.h"

GLFWwindow* glob_glfw_window = nullptr;


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    gltViewport(width, height);
    glViewport(0, 0, width, height);
}

void CloseGameWindow() {
    glfwSetWindowShouldClose(glob_glfw_window, true);
}


std::string Vec3ToStr(const glm::vec3& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
std::string Vec2ToStr(const glm::vec2& vec) {
    return std::to_string(vec.x) + " " + std::to_string(vec.y);
}

// returns the current GLFW time
double GetTime() {
    return glfwGetTime();
}

static f32 deltaTime = 0.0f;
static f32 lastFrameTime = 0.0f;

f32 GetDeltaTime() {
    return deltaTime;
}

static u32 frameCount = 0;
u32 GetFrameCount() { return frameCount; }

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow() {
    glfwSwapBuffers(glob_glfw_window);
    glfwPollEvents();

    // update deltatime
    f32 currentTime = GetTime();
    deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    frameCount++;

    // clear gl buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return glfwWindowShouldClose(glob_glfw_window);
}

void TerminateGame() {
    gltTerminate();
    glfwTerminate();
}

void InitGame(u32 windowWidth, u32 windowHeight, const s8* windowName) {
    s8 cwd[PATH_MAX];
    std::cout << "CWD: " << getcwd(cwd, PATH_MAX) << "\n";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowName, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }    

    // Initialize glText
    gltInit();

    glViewport(0, 0, windowWidth, windowHeight);
    gltViewport(windowWidth, windowHeight);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glob_glfw_window = window;

    Camera::GetMainCamera().screenWidth = windowWidth;
    Camera::GetMainCamera().screenHeight = windowHeight;
}


void EnableWireframeDrawing() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}


#endif