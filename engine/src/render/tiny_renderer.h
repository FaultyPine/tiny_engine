#ifndef TINY_RENDERER_H
#define TINY_RENDERER_H

#include "math/tiny_math.h"

/*
WIP
Notes:

api thoughts
PushXXXX for client code

PushMesh
PushModel
PushPlane
PushPoint
PushTriangle
PushLine (3d and 2d variants?)
PushCircle (2D)
PushSquare (2D)
PushSphere/Cube/ShapeX
^ corresponding PushSphereWire -- these are all refactors for shapes.cpp

Each of these would also take in the Shader (and optional Material) that they're drawn with.

Should Text (world space and screen space) be a part of this?

Should there be some way to automatically draw things in screen space?


Renderer would store Shaders for depth/other prepasses. Would compile variants for those prepasses.... WHEN? HOW?

PrepForDraw() would be called before the render passes that does some sorting for materials and batches some vertex lists together
DrawToFramebuffer(Framebuffer) would do the actual drawing and flush out all queued draw commands/data

Current forward rendering passes:
Shadow pass for directional/point lights
Depth & Normals pass for main camera
Color pass


-- TODO:
- start with doing PushCircle/PushSquare/PushPlane/all the push methods for simple shapes.
    this should let me get a sense of the structure of things without diving into 
    meshes and models and batching and materials and passes and all that


*/
struct Arena;
void InitializeRenderer(Arena* arena);

void RendererDraw();

void PushPoint(glm::vec3 point, glm::vec4 color = glm::vec4(1));
void PushLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1));


#endif