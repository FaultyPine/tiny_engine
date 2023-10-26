#ifndef TINY_SHAPES_H
#define TINY_SHAPES_H

//#include "pch.h"
#include "mesh.h"
struct Shader;

namespace Shapes3D {

void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = {1.0,1.0,1.0,1.0}, f32 width = 1.0);
void DrawCube(const Transform& tf, const glm::vec4& color = glm::vec4(1));
void DrawPlane(const Transform& tf, const glm::vec4& color = glm::vec4(1));

Mesh GenCubeMesh();
Mesh GenPlaneMesh(u32 resolution = 1);

} // namespace Shapes3D

namespace Shapes2D {

void DrawShape(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, const Shader& shader);


void DrawCircle(const glm::vec2& pos, f32 radius, const glm::vec4& color = glm::vec4(1), bool isHollow = false);

void DrawWireframeSquare(const glm::vec2& start, const glm::vec2& end, glm::vec4 color, f32 width);
void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotationDegrees, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow = false);

void DrawLine(const glm::vec2& origin, const glm::vec2& dest, const glm::vec4& color = {1.0,1.0,1.0,1.0}, f32 width = 2.0);

} // namespace Shapes2D


#endif