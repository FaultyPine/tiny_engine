#include "tiny_renderer.h"

#include "tiny_alloc.h"
#include "tiny_engine.h"

struct Renderer
{
    
    Arena lines = {};
    Arena points = {};
};

void InitializeRenderer(Arena* arena)
{
    Renderer* rendererMem = (Renderer*)arena_alloc(arena, sizeof(Renderer));
    GetEngineCtx().renderer = rendererMem;
}

Renderer& GetRenderer()
{
    return *GetEngineCtx().renderer;
}

void RendererDraw()
{

}

void PushPoint(glm::vec3 point, glm::vec4 color)
{
    Renderer& renderer = GetRenderer();
    
}

void PushLine(glm::vec3 start, glm::vec3 end, glm::vec4 color)
{
    Renderer& renderer = GetRenderer();

}