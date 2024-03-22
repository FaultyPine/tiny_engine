#ifndef TINY_SHAPES_H
#define TINY_SHAPES_H

//#include "pch.h"
#include "mesh.h"
struct Shader;

namespace Shapes3D {

// TODO: replace these with renderer equivalents that batch

// TODO: these shouldn't take in Transforms...
// should take in raw position, (opt) rotation, (opt) scale
TAPI void DrawCube(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawWireCube(BoundingBox box, const glm::vec4& color = glm::vec4(1));
TAPI void DrawPlane(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawWirePlane(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawSphere(glm::vec3 center, f32 radius, glm::vec4 color = glm::vec4(1));
TAPI void DrawWireSphere(glm::vec3 center, f32 radius, glm::vec4 color = glm::vec4(1));

TAPI Mesh GenCubeMesh();
TAPI Mesh GenPlaneMesh(u32 resolution = 1);
TAPI Mesh GenSphereMesh(u32 resolution = 16);

} // namespace Shapes3D

namespace Shapes2D {

TAPI void DrawShape(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, const Shader& shader);


TAPI void DrawCircle(
    const glm::vec2& pos, 
    f32 radius, 
    const glm::vec4& color = glm::vec4(1), 
    bool isHollow = false, 
    f32 outlineThickness = 0.07f);

TAPI void DrawWireframeSquare(const glm::vec2& start, const glm::vec2& end, glm::vec4 color);
TAPI void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotationDegrees, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow = false);

} // namespace Shapes2D


#endif