#ifndef OPENGL_WRAPPER_H
#define OPENGL_WRAPPER_H

//#include "pch.h"
#include "tiny_defines.h"
#include "mem/tiny_arena.h"

#define TARGET_FPS 60

struct GLFWwindow;
struct Framebuffer;

typedef void(*InitFunction)(Arena* gameMem);
typedef void(*TickFunction)(Arena* gameMem, f32 deltaTime);
// TODO: how to architect the interplay between the editor and game.
// editor needs the game's callbacks, game needs to render using engine renderer
// editor doesn't really need to render with engine renderer.....
// should the game call the editor and supply it's callbacks that way?
// render functions 
typedef Framebuffer(*RenderFunction)(const Arena* const gameMem);
typedef void(*TerminateFunction)(Arena* gameMem);

struct AppRunCallbacks
{
    InitFunction initFunc = 0;
    TickFunction tickFunc = 0;
    RenderFunction renderFunc = 0;
    TerminateFunction terminateFunc = 0;
};

struct GlobalShaderState;
struct LightingSystem;
struct TextureCache;
struct PhysicsWorld;
struct MaterialRegistry;
struct RendererData;
struct EntityRegistry;
struct PostprocessingSystem;

struct EngineContext
{
    u32 windowWidth = 0;
    u32 windowHeight = 0; 
    u32 aspectRatioW = 0; 
    u32 aspectRatioH = 0;
    const char* appName = nullptr;
    bool false2DTrue3D = false;
    AppRunCallbacks appCallbacks = {};

    const char* resourceDirectory = nullptr;
    Arena gameArena = {};
    Arena engineArena = {}; // persistent
    Arena engineFrameAllocator = {}; // cleared at the end of each frame
    Arena engineSceneAllocator = {}; // persistent for a scene

    f32 deltaTime = 0.0f;
    f32 lastFrameTime = 0.0f;
    u32 frameCount = 0;
    GLFWwindow* glob_glfw_window = nullptr;
    u64 randomSeed = 0;

    // engine subsystems
    GlobalShaderState* shaderSubsystem = 0;
    LightingSystem* lightsSubsystem = 0;
    TextureCache* textureCache = 0;
    PhysicsWorld* physicsWorld = 0;
    MaterialRegistry* materialRegistry = 0;
    RendererData* renderer = 0;
    EntityRegistry* entityRegistry = 0;
    PostprocessingSystem* postprocessingSystem = 0;
};

TAPI EngineContext& GetEngineCtx();

TAPI void InitEngine(
    const char* resourceDirectory,
    const char* windowName,
    u32 windowWidth,
    u32 windowHeight,
    u32 aspectRatioW,
    u32 aspectRatioH,
    bool false2DTrue3D,
    AppRunCallbacks callbacks,
    size_t requestedGameMemSize
);

TAPI void TerminateGame();

TAPI void CloseGameWindow();
TAPI void SetMode2D();
TAPI void SetMode3D();

// returns the current GLFW time
TAPI f64 GetTime();
// just casts GetTime to f32
TAPI f32 GetTimef();

TAPI void OverwriteRandomSeed(u64 seed);
TAPI u64 GetRandomSeed();
TAPI s32 GetRandom(s32 start, s32 end);
TAPI f32 GetRandomf(f32 start, f32 end);

TAPI f32 GetDeltaTime();

TAPI u32 GetFrameCount();

TAPI void ClearGLBuffers();
TAPI void SetMinAndMaxWindowSize(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
TAPI void ClearGLColorBuffer();

TAPI void SetWireframeDrawing(bool shouldDrawWireframes);

TAPI u32 HashBytes(u8* data, u32 size);
TAPI u64 HashBytesL(u8* data, u32 size);

TAPI Arena* GetSceneAllocator();
TAPI Arena* GetFrameAllocator();

struct GLFWwindow;
TAPI GLFWwindow* GetMainGLFWWindow();

#endif