#include "tiny_engine.h"

#include "tiny_fs.h"
#include "tiny_text.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "input.h"
#include "sprite.h"
#include "math.h"
#include "tiny_profiler.h"


static f32 deltaTime = 0.0f;
static f32 lastFrameTime = 0.0f;
static u32 frameCount = 0;
GLFWwindow* glob_glfw_window = nullptr;
static u64 randomSeed = 0;

void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height) {
    s32 screenWidth = width;
    s32 screenHeight = height;
    UpdateGLTViewport(screenWidth, screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    Camera::GetMainCamera().screenWidth = screenWidth;
    Camera::GetMainCamera().screenHeight = screenHeight;
}
void TerminateGame() {
    GLTTerminate();
    glfwTerminate();
    Profiler::Instance().endSession();
}
void OverwriteRandomSeed(u64 seed) {
    randomSeed = seed;
}
u64 GetRandomSeed() { return randomSeed; }
s32 GetRandom(s32 start, s32 end) {
    // lazy init random seed
    if (randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "\n";
    }
    srand(Math::hash((const char*)&randomSeed, sizeof(randomSeed)));
    randomSeed++; // deterministic random
    return start + (rand() % end);
}
f32 GetRandomf(f32 start, f32 end) {
    // lazy init random seed
    if (randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "\n";
    }
    srand(Math::hash((const char*)&randomSeed, sizeof(randomSeed)));
    randomSeed++; // deterministic random
    f32 zeroToOneRandom = ((f32)rand()) / RAND_MAX;
    return Math::Lerp(start, end, zeroToOneRandom);
}

// returns the current GLFW time (in seconds)
f64 GetTime() {
    return glfwGetTime();
}
f32 GetTimef() {
    return (f32)GetTime();
}
void CloseGameWindow() {
    glfwSetWindowShouldClose(glob_glfw_window, true);
}
f32 GetDeltaTime() {
    return deltaTime;
}
u32 GetFrameCount() { return frameCount; }

void ClearGLBuffers() {
    // clear gl buffer
    ClearGLColorBuffer();
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void ClearGLColorBuffer() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight) {
    glfwSetWindowSizeLimits(glob_glfw_window, minWidth, minHeight, maxWidth, maxHeight);
    Camera& cam = Camera::GetMainCamera();
    cam.minScreenWidth = minWidth;
    cam.minScreenHeight = minHeight;
    cam.maxScreenWidth = maxWidth;
    cam.maxScreenHeight = maxHeight;
}

void EngineLoop() {
    // update deltatime
    f32 currentTime = GetTime();
    deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    // inc frame
    frameCount++;
    // update cam
    Camera::UpdateCamera();

    { // sleep until we should draw the next frame
        static f64 lastframe = GetTime();
        while (GetTime() < lastframe + 1.0/TARGET_FPS) {
            // zzzzzz
        }
        lastframe += 1.0/TARGET_FPS;
    }
    // clear screen after we've waited
    ClearGLBuffers();
}

/// Game loop should be while(!ShouldCloseWindow())
bool ShouldCloseWindow() {
    glfwSwapBuffers(glob_glfw_window);
    glfwPollEvents();
    EngineLoop();
    return glfwWindowShouldClose(glob_glfw_window);
}

void InitGame(u32 windowWidth, u32 windowHeight, u32 aspectRatioW, u32 aspectRatioH, const s8* windowName) {
    Profiler::Instance().beginSession("Profile");

    s8 cwd[PATH_MAX];
    #ifdef _MSC_VER
    std::cout << "CWD: " << getcwd(cwd, PATH_MAX) << "\n";
    #else
    std::cout << "CWD: " << getcwd(cwd, PATH_MAX) << "\n";
    #endif

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
#ifdef TINY_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
#ifdef __APPLE__
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
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
    if (!GLTInitialize()) {
        std::cout << "[ERROR] Failed to initialize GLtext!\n";
    }

    glViewport(0, 0, windowWidth, windowHeight);
    UpdateGLTViewport(windowWidth, windowHeight);
    
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    glfwSetWindowAspectRatio(window, aspectRatioW, aspectRatioH);

    // For 2D games, don't depth test so that the order they are drawn in makes sense
    // (subsequent draws overwrite previous draws)
    //glDepthFunc(GL_NEVER);
    // comment out above line and comment in below line for proper 3D depth testing
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_STENCIL_TEST);  
   

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);

    glob_glfw_window = window;

    Camera::GetMainCamera().screenWidth = windowWidth;
    Camera::GetMainCamera().screenHeight = windowHeight;
}

void SetWireframeDrawing(bool shouldDrawWireframes) {
    glPolygonMode(GL_FRONT_AND_BACK, shouldDrawWireframes ? GL_LINE : GL_FILL);
}