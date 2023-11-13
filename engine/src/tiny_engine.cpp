//#include "pch.h"
#include "tiny_engine.h"

#include "tiny_fs.h"
#include "render/tiny_text.h"
#include "render/shader.h"
#include "camera.h"
#include "render/mesh.h"
#include "render/texture.h"
#include "input.h"
#include "render/sprite.h"
#include "math/tiny_math.h"
#include "tiny_profiler.h"
#include "render/tiny_ogl.h"
#include "render/framebuffer.h"
#include "tiny_log.h"
#include "tiny_alloc.h"
#include "tiny_imgui.h"

#include "GLFW/glfw3.h"

// for getcwd
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <direct.h>
#endif

STATIC_ASSERT(sizeof(s8) == 1);
STATIC_ASSERT(sizeof(u8) == 1);
STATIC_ASSERT(sizeof(s16) == 2);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(s32) == 4);
STATIC_ASSERT(sizeof(u64) == 8);
STATIC_ASSERT(sizeof(s64) == 8);

STATIC_ASSERT(sizeof(f32) == 4);
STATIC_ASSERT(sizeof(f64) == 8);

STATIC_ASSERT(sizeof(void*) == 8);

static EngineState globEngineState;

GLFWwindow* GetMainGLFWWindow() { return globEngineState.glob_glfw_window; }


void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height) {
    s32 screenWidth = width;
    s32 screenHeight = height;
    UpdateGLTViewport(screenWidth, screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    Camera::GetMainCamera().screenWidth = screenWidth;
    Camera::GetMainCamera().screenHeight = screenHeight;
}
void TerminateGame() {
    globEngineState.frameCount = 0;
    globEngineState.deltaTime = 0;
    globEngineState.lastFrameTime = 0;
    globEngineState.randomSeed = 0;

    GLTTerminate();
    glfwTerminate();
    Profiler::Instance().endSession();
}
void OverwriteRandomSeed(u64 seed) {
    globEngineState.randomSeed = seed;
}
u64 GetRandomSeed() { return globEngineState.randomSeed; }
s32 GetRandom(s32 start, s32 end) {
    // lazy init random seed
    if (globEngineState.randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        globEngineState.randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "";
    }
    srand(Math::hash((const char*)&globEngineState.randomSeed, sizeof(globEngineState.randomSeed)));
    globEngineState.randomSeed++; // deterministic random
    return start + (rand() % end);
}
f32 GetRandomf(f32 start, f32 end) {
    // lazy init random seed
    if (globEngineState.randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        globEngineState.randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "";
    }
    srand(Math::hash((const char*)&globEngineState.randomSeed, sizeof(globEngineState.randomSeed)));
    globEngineState.randomSeed++; // deterministic random
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
    glfwSetWindowShouldClose(GetMainGLFWWindow(), true);
}
f32 GetDeltaTime() {
    return globEngineState.deltaTime;
}
u32 GetFrameCount() { return globEngineState.frameCount; }

void SetMode2D() {
    // For 2D games, don't depth test so that the order they are drawn in makes sense
    // (subsequent draws overwrite previous draws)
    glDepthFunc(GL_NEVER);
    Camera::GetMainCamera().SetMode2D();
}
void SetMode3D() {
    glEnable(GL_DEPTH_TEST);
    Camera::GetMainCamera().SetMode3D();
}

void ClearGLBuffers() {
    // clear gl buffer
    ClearGLColorBuffer();
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
void ClearGLColorBuffer() {
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight) {
    glfwSetWindowSizeLimits(GetMainGLFWWindow(), minWidth, minHeight, maxWidth, maxHeight);
    Camera& cam = Camera::GetMainCamera();
    cam.minScreenWidth = minWidth;
    cam.minScreenHeight = minHeight;
    cam.maxScreenWidth = maxWidth;
    cam.maxScreenHeight = maxHeight;
}

void SetWireframeDrawing(bool shouldDrawWireframes) {
    glPolygonMode(GL_FRONT_AND_BACK, shouldDrawWireframes ? GL_LINE : GL_FILL);
}

void EngineLoop() {
    // update deltatime
    f32 currentTime = GetTime();
    globEngineState.deltaTime = currentTime - globEngineState.lastFrameTime;
    globEngineState.lastFrameTime = currentTime;
    // inc frame
    globEngineState.frameCount++;
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
    glfwSwapBuffers(GetMainGLFWWindow());
    glfwPollEvents();
    EngineLoop();
    return glfwWindowShouldClose(GetMainGLFWWindow());
}

void InitEngine(const EngineState& engineInitState, size_t requestedMemSize, int argc, char *argv[])
{
    InitializeLogger();
    globEngineState = engineInitState;
    u32 windowWidth = engineInitState.windowWidth;
    u32 windowHeight = engineInitState.windowHeight;
    u32 aspectRatioW = engineInitState.aspectRatioW;
    u32 aspectRatioH = engineInitState.aspectRatioH;
    const s8* windowName = engineInitState.appName;
    bool false2DTrue3D = engineInitState.false2DTrue3D;

    if (argc < 2)
    {
        LOG_FATAL("Passed too few arguments to engine. Make sure to specify the resource directory");
    }
    const char* resourceDirectory = argv[1];
    globEngineState.resourceDirectory = resourceDirectory;

    TINY_ASSERT(globEngineState.resourceDirectory);
    AppRunCallbacks& gameFuncs = globEngineState.appCallbacks;
    TINY_ASSERT(gameFuncs.initFunc);
    TINY_ASSERT(gameFuncs.tickFunc);
    TINY_ASSERT(gameFuncs.renderFunc);
    TINY_ASSERT(gameFuncs.terminateFunc);

    Profiler::Instance().beginSession("Profile");
    InitializeTinyFilesystem(resourceDirectory);
    LOG_INFO("Resource directory: %s", resourceDirectory);

    s8 cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    LOG_INFO("CWD: %s", cwd);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4); // ask for multisampled buffers so we can do multisampling if we want
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
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    globEngineState.glob_glfw_window = window;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return;
    }    
    LOG_INFO("OpenGL Driver Version: %s", glGetString(GL_VERSION));

    // Initialize glText
    if (!GLTInitialize()) {
        LOG_ERROR("Failed to initialize GLtext!");
    }

    glViewport(0, 0, windowWidth, windowHeight);
    UpdateGLTViewport(windowWidth, windowHeight);
    setCursorMode(CursorMode::DISABLED); // lock cursor into window
    glfwSetWindowAspectRatio(window, aspectRatioW, aspectRatioH);

    if (false2DTrue3D) {
        SetMode3D();
    }
    else {
        SetMode2D();
    }

    glEnable(GL_STENCIL_TEST);  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    // slope scale depth bias (helps prevent shadow artifacts (acne) in hardware)
    //glEnable(GL_POLYGON_OFFSET_FILL); 
    //glPolygonOffset(1.0f, 1.0f);

#ifdef TINY_DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OglDebugMessageCallback, 0);
#endif

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);

    Camera::GetMainCamera().screenWidth = windowWidth;
    Camera::GetMainCamera().screenHeight = windowHeight;

    InitImGui();

    // give a big memory pool to the game. Game shouldn't allocate outside this pool
    void* gameMemory = TALLOC(requestedMemSize);
    globEngineState.gameArena = arena_init(gameMemory, requestedMemSize);
    Arena* gameArena = &globEngineState.gameArena;

    // give some of our memory to shaders to store uniforms
    size_t uniformsMemBlockSize = MEGABYTES_BYTES(2);
    TINY_ASSERT(uniformsMemBlockSize < requestedMemSize);
    InitializeShaderSystem(gameArena, uniformsMemBlockSize);

    // Begin game loop
    gameFuncs.initFunc(gameArena);
    while (!ShouldCloseWindow())
    {
        gameFuncs.tickFunc(gameArena);
        ImGuiBeginFrame();
        Framebuffer screenRenderFb = gameFuncs.renderFunc(gameArena);
        glm::vec2 screen = glm::vec2(Camera::GetScreenWidth(), Camera::GetScreenHeight());
        Framebuffer::Blit(screenRenderFb.framebufferID, 0, 0, screen.x, screen.y, 0, 0, 0, screen.x, screen.y, Framebuffer::FramebufferAttachmentType::COLOR);
        ImGuiEndFrame();
    }
    gameFuncs.terminateFunc(gameArena);

    arena_free_all(gameArena);
    ImGuiTerminate();
    TerminateGame();
}