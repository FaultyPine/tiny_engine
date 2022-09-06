#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

#include "pch.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "input.h"

GLFWwindow* glob_glfw_window = nullptr;

struct GameState {
    Mesh objs[1];
    Camera camera;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void CloseGameWindow() {
    glfwSetWindowShouldClose(glob_glfw_window, true);
}

/// appends resource path to provided path
std::string UseResPath(const std::string& path = "") {
    return "../src/res/" + path;
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

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow() {
    glfwSwapBuffers(glob_glfw_window);
    glfwPollEvents();

    // update deltatime
    f32 currentTime = GetTime();
    deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    // clear gl buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return glfwWindowShouldClose(glob_glfw_window);
}

void TerminateGame(GameState& gs) {
    glfwTerminate();
}

void InitWindow(u32 windowWidth, u32 windowHeight, const s8* windowName) {
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

    glViewport(0, 0, windowWidth, windowHeight);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);

    glob_glfw_window = window;
}


void EnableWireframeDrawing() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}


#endif