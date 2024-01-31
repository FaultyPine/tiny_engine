#ifndef TINY_SHAPES_H
#define TINY_SHAPES_H

//#include "pch.h"
#include "mesh.h"
struct Shader;

namespace Shapes3D {

// TODO: these shouldn't take in Transforms...
// should take in raw position, (opt) rotation, (opt) scale
TAPI void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = {1.0,1.0,1.0,1.0}, f32 width = 1.0);
TAPI void DrawCube(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawWireCube(BoundingBox box, const glm::vec4& color = glm::vec4(1));
TAPI void DrawPlane(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawWirePlane(const Transform& tf, const glm::vec4& color = glm::vec4(1));
TAPI void DrawSphere(glm::vec3 center, f32 radius, glm::vec4 color = glm::vec4(1));
TAPI void DrawWireSphere(glm::vec3 center, f32 radius, glm::vec4 color = glm::vec4(1));
TAPI void DrawPoint(glm::vec3 position, f32 size = 1.0f, glm::vec4 color = glm::vec4(1));
TAPI void DrawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec4 color = glm::vec4(1));

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

TAPI void DrawWireframeSquare(const glm::vec2& start, const glm::vec2& end, glm::vec4 color, f32 width);
TAPI void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotationDegrees, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow = false);

TAPI void DrawLine(const glm::vec2& origin, const glm::vec2& dest, const glm::vec4& color = {1.0,1.0,1.0,1.0}, f32 width = 2.0);

} // namespace Shapes2D


#endif