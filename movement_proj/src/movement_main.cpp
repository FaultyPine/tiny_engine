//#include "pch.h"
#include "movement_main.h"
#include "tiny_log.h"
#include "tiny_engine.h"
#include "render/framebuffer.h"
#include "render/shapes.h"

#include "solver.h"

constexpr u32 SCREEN_WIDTH = 1280;
constexpr u32 SCREEN_HEIGHT = 720;
constexpr u32 NUM_MAX_BALLS = 50;

struct Ball
{
    glm::vec4 color = glm::vec4(1.0, 0.0, 0.0, 1.0);
};

struct MvGameState
{
    Framebuffer mainFb = {};
    // physics object array for simulation
    PhysicsObject* objs = nullptr;
    // Ball list for rendering
    Ball* balls = nullptr;
    // both renderable balls and physicsobjects size themselves based on this
    // index 0 into objs should correspond to the same index in balls
    u32 numBalls = 1;
};

void movement_init(Arena* gameMem)
{
    MvGameState* gs = (MvGameState*)arena_alloc(gameMem, sizeof(MvGameState));
    gs->numBalls = NUM_MAX_BALLS;
    gs->objs = (PhysicsObject*)arena_alloc(gameMem, sizeof(PhysicsObject) * gs->numBalls);
    gs->balls = (Ball*)arena_alloc(gameMem, sizeof(Ball) * gs->numBalls);
    for (u32 i = 0; i < gs->numBalls; i++)
    {
        gs->balls[i] = Ball(); // explicit default init since we manually alloc
        gs->objs[i] = PhysicsObject();
    }
    gs->mainFb = Framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void movement_tick(Arena* gameMem)
{
    MvGameState* gs = (MvGameState*)gameMem->backing_mem;
    f32 dt = GetDeltaTime();
    Solver::update(gs->objs, gs->numBalls, dt);
}

void render_ball(MvGameState* gs, u32 ballIdx)
{
    PhysicsObject& obj = gs->objs[ballIdx];
    Ball& ball = gs->balls[ballIdx];
    Shapes2D::DrawCircle(obj.position, obj.radius, ball.color, false);
}

Framebuffer movement_render(const Arena* const gameMem)
{
    MvGameState* gs = (MvGameState*)gameMem->backing_mem;
    Framebuffer& mainFb = gs->mainFb;
    mainFb.Bind();
    ClearGLBuffers();
    
    // draw
    for (u32 i = 0; i < gs->numBalls; i++)
    {
        render_ball(gs, i);
    }

    Shapes2D::DrawCircle(glm::vec2(10.0f, 10.0f), 100.0f, glm::vec4(1.0, 1.0, 1.0, 1.0), true, 0.01f);

    Framebuffer::BindDefaultFrameBuffer();
    return mainFb;
}

void movement_terminate(Arena* gameMem)
{
    MvGameState* gs = (MvGameState*)gameMem->backing_mem;
    gs->mainFb.Delete();
}

AppRunCallbacks GetMovementProjAppRunCallbacks()
{
    AppRunCallbacks out;
    out.initFunc = movement_init;
    out.tickFunc = movement_tick;
    out.renderFunc = movement_render;
    out.terminateFunc = movement_terminate;
    return out;
}

void movement_standalone_entrypoint(int argc, char *argv[])
{
    char* resourceDirectory = "./res/";
    if (argc < 2)
    {
        LOG_WARN("no resource directory passed. Using default ./res/");
    }
    else
    {
        resourceDirectory = argv[1];
    }
    InitEngine(
        resourceDirectory,
        "Movement",
        SCREEN_WIDTH, SCREEN_HEIGHT,
        16, 9,
        false,
        GetMovementProjAppRunCallbacks(), 
        MEGABYTES_BYTES(2)
    ); 
}