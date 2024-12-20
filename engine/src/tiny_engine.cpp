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
#include "mem/tiny_arena.h"
#include "tiny_imgui.h"
#include "render/tiny_lights.h"
#include "physics/tiny_physics.h"
#include "job_system.h"
#include "render/tiny_renderer.h"
#include "scene/entity.h"
#include "tiny_thread.h"
#include "render/postprocess.h"

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

static EngineContext globEngineCtx = {};

EngineContext& GetEngineCtx() { return globEngineCtx; }

GLFWwindow* GetMainGLFWWindow() { return globEngineCtx.glob_glfw_window; }


void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height) {
    s32 screenWidth = width;
    s32 screenHeight = height;
    UpdateGLTViewport(screenWidth, screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    Camera::GetMainCamera().screenWidth = screenWidth;
    Camera::GetMainCamera().screenHeight = screenHeight;
}
void TerminateGame() {
    globEngineCtx.frameCount = 0;
    globEngineCtx.deltaTime = 0;
    globEngineCtx.lastFrameTime = 0;
    globEngineCtx.randomSeed = 0;

    GLTTerminate();
    glfwTerminate();
}
void OverwriteRandomSeed(u64 seed) {
    globEngineCtx.randomSeed = seed;
}
u64 GetRandomSeed() { return globEngineCtx.randomSeed; }
s32 GetRandom(s32 start, s32 end) {
    // lazy init random seed
    if (globEngineCtx.randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        globEngineCtx.randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "";
    }
    srand(Math::hash((const char*)&globEngineCtx.randomSeed, sizeof(globEngineCtx.randomSeed)));
    globEngineCtx.randomSeed++; // deterministic random
    return start + (rand() % end);
}
f32 GetRandomf(f32 start, f32 end) {
    // lazy init random seed
    if (globEngineCtx.randomSeed == 0) {
        // truly random initial seed. Subsequent random calls simply increment the seed deterministically
        f64 time = GetTime();
        globEngineCtx.randomSeed = Math::hash((const char*)&time, sizeof(f64));
        //std::cout << "Initial random seed = " << randomSeed << "";
    }
    // deterministic random
    u32 newSeed = Math::hash((const char*)&globEngineCtx.randomSeed, sizeof(globEngineCtx.randomSeed));
    srand(newSeed);
    globEngineCtx.randomSeed++; 
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
    return globEngineCtx.deltaTime;
}
u32 GetFrameCount() { return globEngineCtx.frameCount; }

void SetMode2D() {
    // For 2D games, don't depth test so that the order they are drawn in makes sense
    // (subsequent draws overwrite previous draws)
    glDepthFunc(GL_NEVER);
    Camera::GetMainCamera().SetMode2D();
}
void SetMode3D() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
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

u64 HashBytesL(u8* data, u32 size)
{
    // FNV-1 hash
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    constexpr u64 FNV_offset_basis = 0xcbf29ce484222325;
    constexpr u64 FNV_prime = 0x100000001b3;
    u64 hash = FNV_offset_basis;
    for (u32 i = 0; i < size; i++)
    {
        u8 byte_of_data = data[i];
        hash = hash * FNV_prime;
        hash = hash ^ byte_of_data;
    }
    return hash;
}
u32 HashBytes(u8* data, u32 size)
{
    // FNV-1 hash -> doing the *PRIME, and THEN the XOR.
    // FNV-1a hash -> doing the XOR, then the *PRIME
    // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    constexpr u32 FNV_offset_basis = 0x811c9dc5;
    constexpr u32 FNV_prime = 0x01000193;
    u32 hash = FNV_offset_basis;
    for (u32 i = 0; i < size; i++)
    {
        u8 byte_of_data = data[i];
        hash = hash ^ byte_of_data;
        hash = hash * FNV_prime;
    }
    return hash;
}

Arena* GetSceneAllocator()
{
    return &globEngineCtx.engineSceneAllocator;
}

Arena* GetFrameAllocator()
{
    return &globEngineCtx.engineFrameAllocator;
}

/// Game loop - while(EngineLoop())
bool EngineLoop() {
    PROFILE_FUNCTION();
    { PROFILE_SCOPE("Glfw Swap Buffers");
        glfwSwapBuffers(GetMainGLFWWindow());
        PROFILER_GPU_FLUSH();
    }
    if (Keyboard::isKeyDown(TINY_KEY_ESCAPE)) {
        CloseGameWindow();
    }
    f32 currentTime = GetTimef();
    globEngineCtx.deltaTime = currentTime - globEngineCtx.lastFrameTime;
    globEngineCtx.lastFrameTime = currentTime;
    globEngineCtx.frameCount++;
    Camera::UpdateCamera();
    ClearGLBuffers();
    { // sleep until we should draw the next frame
        PROFILE_SCOPE("WaitForNextFrame");
        static f64 lastframe = GetTime();
        f64 targetFrametime = 1.0/TARGET_FPS;
        while (GetTime() < lastframe + targetFrametime) 
        {
            // zzzzzz
            std::this_thread::yield();
        }
        lastframe += targetFrametime;
    }
    glfwPollEvents();
    return glfwWindowShouldClose(GetMainGLFWWindow());
}

void InitEngine(
    const char* resourceDirectory,
    const char* windowName,
    u32 windowWidth,
    u32 windowHeight,
    u32 aspectRatioW,
    u32 aspectRatioH,
    bool false2DTrue3D,
    AppRunCallbacks callbacks,
    size_t requestedGameMemSize
) {
    InitializeLogger();
    TINY_ASSERT(resourceDirectory);
    globEngineCtx.resourceDirectory = resourceDirectory;

    s8 cwd[PATH_MAX];
    #ifdef _WIN32
    _getcwd(cwd, PATH_MAX);
    #else
    getcwd(cwd, PATH_MAX);
    #endif
    LOG_INFO("CWD: %s", cwd);
    LOG_INFO("Resource directory: %s", resourceDirectory);

    SetThreadName("TinyEngine Main Thread");

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
    globEngineCtx.glob_glfw_window = window;

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
    PROFILER_GPU_CONTEXT(); 
#endif

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize engine

    Camera::GetMainCamera().screenWidth = windowWidth;
    Camera::GetMainCamera().screenHeight = windowHeight;

    InitImGui();

    // engine memory
    u32 engineMemorySize = MEGABYTES_BYTES(100); // :/
    void* engineMemory = TSYSALLOC(engineMemorySize);
    TMEMSET(engineMemory, 0, engineMemorySize);
    globEngineCtx.engineArena = arena_init(engineMemory, engineMemorySize, "Engine");
    Arena* engineArena = &globEngineCtx.engineArena;
    // scene allocator gets some % of engine mem
    constexpr u32 engineMemoryPercentSceneAllocator = 40;
    u32 engineSceneAllocatorMemSize = Math::PercentOf(engineMemorySize, engineMemoryPercentSceneAllocator);
    // frame allocator gets some % of engine mem
    constexpr u32 engineMemoryPercentFrameAllocator = 40;
    u32 engineFrameAllocatorMemSize = Math::PercentOf(engineMemorySize, engineMemoryPercentFrameAllocator);
    globEngineCtx.engineSceneAllocator = arena_init(arena_alloc(engineArena, engineSceneAllocatorMemSize), engineSceneAllocatorMemSize, "Engine Scene");
    globEngineCtx.engineFrameAllocator = arena_init(arena_alloc(engineArena, engineFrameAllocatorMemSize), engineFrameAllocatorMemSize, "Engine Frame");

    // subsystem initialization
    JobSystem::Instance().Initialize();
    InitializeTinyFilesystem(resourceDirectory);
    InitializeShaderSystem(engineArena);
    InitializeLightingSystem(engineArena);
    InitializeTextureCache(engineArena);
    InitializeMaterialSystem(engineArena);
    Renderer::InitializeRenderer(engineArena);
    Postprocess::InitializePostprocessing(engineArena);
    Entity::InitializeEntitySystem(engineArena);
    InitializePhysics(engineArena);


    TINY_ASSERT(globEngineCtx.resourceDirectory);
    globEngineCtx.appCallbacks = callbacks;
    AppRunCallbacks& gameFuncs = globEngineCtx.appCallbacks;
    TINY_ASSERT(gameFuncs.initFunc);
    TINY_ASSERT(gameFuncs.tickFunc);
    TINY_ASSERT(gameFuncs.renderFunc);
    TINY_ASSERT(gameFuncs.terminateFunc);

    

    // give a big memory pool to the game. Game shouldn't allocate outside this pool
    void* gameMemory = TSYSALLOC(requestedGameMemSize);
    TMEMSET(gameMemory, 0, requestedGameMemSize);
    globEngineCtx.gameArena = arena_init(gameMemory, requestedGameMemSize, "Game");
    Arena* gameArena = &globEngineCtx.gameArena;

    { PROFILE_SCOPE("Game Initialize");
        gameFuncs.initFunc(gameArena);
    }

    // game loop
    while (!EngineLoop())
    {
        PROFILER_FRAME_MARK();
        ImGuiBeginFrame();
        PhysicsTick();
        { PROFILE_SCOPE("Game tick");
            gameFuncs.tickFunc(gameArena, GetDeltaTime());
        }
        JobSystem::Instance().FlushMainThreadJobs();
        { PROFILE_SCOPE("Engine Render");
            { PROFILE_SCOPE("Game Render");
                gameFuncs.renderFunc(gameArena);
            }
            Framebuffer* finalpass = Renderer::RendererDraw();
            glm::vec2 screen = glm::vec2(Camera::GetScreenWidth(), Camera::GetScreenHeight());
            // blit the game's rendered frame to default framebuffer
            if (finalpass->isValid())
            {
                constexpr u32 defaultFramebufferID = 0;
                Framebuffer::Blit(
                    finalpass->framebufferID, 0, 0, screen.x, screen.y, 
                    defaultFramebufferID, 0, 0, screen.x, screen.y);
            }
        }
        arena_clear(GetFrameAllocator());
        ImGuiEndFrame();
    }
    gameFuncs.terminateFunc(gameArena);

    JobSystem::Instance().Shutdown();
    TerminateGame();
    arena_free_all(gameArena);
    arena_free_all(engineArena);
    ImGuiTerminate();
}