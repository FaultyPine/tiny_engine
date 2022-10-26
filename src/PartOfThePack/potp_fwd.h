#ifndef POTP_FWD
#define POTP_FWD

#include "tiny_engine/pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/math.h"

#define MAX_NUM_PLAYERS 4

#define MIN_WIDTH 800
#define MIN_HEIGHT 600
#define MAX_WIDTH 1300
#define MAX_HEIGHT 975

/// returns the width of the window - minimum width of the window
inline f32 GetWindowAdditionalWidth() {
    f32 width = Camera::GetScreenWidth();
    return width - MIN_WIDTH;
}
/// returns the height of the window - minimum height of the window
inline f32 GetWindowAdditionalHeight() {
    f32 height = Camera::GetScreenHeight();
    return height - MIN_HEIGHT;
}
inline f32 GetWindowWidthScale01() {
    f32 width = Camera::GetScreenWidth();
    return width / MIN_WIDTH;
}

struct CollisionBox {
    glm::vec2 pos;
    glm::vec2 size;
};

struct Entity {
    Sprite sprite = {};
    glm::vec2 position = glm::vec2(0.0f, 0.0f);
    glm::vec2 size = glm::vec2(50.0f, 50.0f);
    f32 rotation = 0.0f;
    glm::vec4 color = glm::vec4(1.0f);
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