#ifndef POTP_FWD
#define POTP_FWD

#include "tiny_engine/pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/math.h"

struct Point2D {
    f32 x = 0.0;
    f32 y = 0.0;
};
struct Color {
    f32 r = 0.0;
    f32 g = 0.0;
    f32 b = 0.0;
};


struct CollisionBox {
    glm::vec2 pos;
    glm::vec2 size;
};

struct Entity {
    Sprite sprite = {};
    glm::vec2 position = glm::vec2(0.0f, 0.0f);
    glm::vec2 size = glm::vec2(50.0f, 50.0f);
    f32 rotation = 0.0f;
    glm::vec3 color = glm::vec3(1.0f);
    bool active = true;
    std::string name = "";
    u32 zIndex = 0;

    inline void Draw() const {
        Camera& cam = Camera::GetMainCamera();
        sprite.DrawSprite(cam, glm::vec2(position.x, position.y), size, rotation, glm::vec3(0.0, 0.0, 1.0), color);
    }
    inline bool isCollidingWith(const Entity& other) const {
        return Math::isOverlappingRect2D(this->position, this->size, other.position, other.size);
    }
};



#endif