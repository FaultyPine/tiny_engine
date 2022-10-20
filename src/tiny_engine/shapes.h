#ifndef TINY_SHAPES_H
#define TINY_SHAPES_H

#include "pch.h"
#include "shader.h"
#include "tiny_fs.h"

namespace Shapes {


void DrawShape(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, const Shader& shader);


void DrawCircle(const glm::vec2& pos, f32 radius, const glm::vec4& color, bool hollow = false);

void DrawSquare(const glm::vec2& pos, const glm::vec2& size, 
            f32 rotation, const glm::vec3& rotationAxis, 
            const glm::vec4& color, bool isHollow = false);

void DrawLine(const glm::vec2 origin, const glm::vec2& dest, const glm::vec4& color, f32 width);

}
#endif