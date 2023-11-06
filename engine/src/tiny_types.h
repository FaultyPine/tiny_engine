#ifndef TINY_TYPES_H
#define TINY_TYPES_H

//#include "pch.h"
#include "math/tiny_math.h"

// TODO: seperate into cpp file

struct Transform {
    glm::vec3 position = glm::vec3(0);
    glm::vec3 scale = glm::vec3(1);
    f32 rotation = 0.0;
    glm::vec3 rotationAxis = {0,1,0};

    Transform(const glm::vec3& pos = glm::vec3(0), const glm::vec3& scl = glm::vec3(1), f32 rot = 0.0, const glm::vec3& rotAxis = {0,1,0}) {
        position = pos;
        scale = scl;
        rotation = rot;
        rotationAxis = rotAxis;
    }

    glm::mat4 ToModelMatrix() const {
        return Math::Position3DToModelMat(position, scale, rotation, rotationAxis);
    }
};
struct Transform2D {
    glm::vec2 position = glm::vec2(0);
    glm::vec2 scale = glm::vec2(1);
    f32 rotation = 0.0;

    Transform2D(const glm::vec2& pos = glm::vec2(0), const glm::vec2& scl = glm::vec2(1), f32 rot = 0.0) {
        position = pos;
        scale = scl;
        rotation = rot;
    }

    glm::mat4 ToModelMatrix() const {
        glm::vec3 pos3D = glm::vec3(position.x, position.y, 0.0f);
        glm::vec3 scl3D = glm::vec3(scale.x, scale.y, 0.0f);
        return Math::Position3DToModelMat(pos3D, scl3D, rotation, {0,0,1});
    }
};

struct BoundingBox {
    BoundingBox() = default;
    BoundingBox(glm::vec3 mn, glm::vec3 mx) {min = mn; max = mx;}
    glm::vec3 min, max = glm::vec3(0);
};

struct BoundingBox2D {
    BoundingBox2D() = default;
    BoundingBox2D(glm::vec2 mn, glm::vec2 mx) {min = mn; max = mx;}
    glm::vec2 min = glm::vec2(0);
    glm::vec2 max = glm::vec2(0);

    bool isInBounds(glm::vec2 point) {
        return (point.x >= min.x && point.x <= max.x
            && point.y >= min.y && point.y <= max.y);
    }
    bool isIntersecting(BoundingBox2D other) {
        return Math::isOverlappingRect2D(min, max, other.min, other.max);
    }
};

#endif