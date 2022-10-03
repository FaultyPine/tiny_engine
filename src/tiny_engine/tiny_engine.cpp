#include "tiny_engine.h"

#include "tiny_fs.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "input.h"
#include "sprite.h"
#include "math.h"

// for rdtsc for rand
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static f32 deltaTime = 0.0f;
static f32 lastFrameTime = 0.0f;
static u32 frameCount = 0;
GLFWwindow* glob_glfw_window = nullptr;
static u64 randomSeed = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    gltViewport(width, height);
    glViewport(0, 0, width, height);
}
void TerminateGame() {
    gltTerminate();
    glfwTerminate();
}
u32 GetRandom(u32 start, u32 end) {
    // lazy init random seed
    if (randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        randomSeed = GetCPUCycles();
        std::cout << "Initial random seed = " << randomSeed << "\n";
    }
    srand(hash((const char*)&randomSeed, sizeof(randomSeed)));
    randomSeed++; // deterministic random
    return start + (rand() % end);
}
long long GetCPUCycles() {
    return __rdtsc();
}

// returns the current GLFW time
double GetTime() {
    return glfwGetTime();
}
long long GetTimeSinceEpoch() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
void CloseGameWindow() {
    glfwSetWindowShouldClose(glob_glfw_window, true);
}
f32 GetDeltaTime() {
    return deltaTime;
}
u32 GetFrameCount() { return frameCount; }

void EngineLoop() {
    // update deltatime
    f32 currentTime = GetTime();
    deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    frameCount++;

    // clear gl buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

}

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow() {
    glfwSwapBuffers(glob_glfw_window);
    glfwPollEvents();
    EngineLoop();
    return glfwWindowShouldClose(glob_glfw_window);
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

    // For 2D games, don't depth test so that the order they are drawn in makes sense
    // (subsequent draws overwrite previous draws)
    glDepthFunc(GL_NEVER);
    // comment out above line and comment in below line for proper 3D depth testing
    //glEnable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST);  
   

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