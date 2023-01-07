#ifndef TINY_TYPES_H
#define TINY_TYPES_H

#include "pch.h"
#include "math.h"

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

struct BoundingBox {
    BoundingBox(){}
    BoundingBox(glm::vec3 mn, glm::vec3 mx) {min = mn; max = mx;}
    glm::vec3 min, max = glm::vec3(0);
};

#endif