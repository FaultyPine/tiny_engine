#ifndef SPACESHOOTER_FWD_H
#define SPACESHOOTER_FWD_H

#include "../tiny_engine/pch.h"
#include "../tiny_engine/sprite.h"
#include "../tiny_engine/math.h"

struct Entity {
    Sprite sprite;
    glm::vec3 position;
    glm::vec2 size = glm::vec2(10.0f, 10.0f);
    f32 rotation;
    glm::vec3 color = glm::vec3(1.0f);
    bool active = true;
    std::string name = "";

    inline void Draw(const Camera& cam) {
        sprite.DrawSprite(cam, glm::vec2(position.x, position.y), size, rotation, color);
    }
    inline bool isCollidingWith(const Entity& other) const {
        return Math::isOverlappingRect2D(this->position, this->size, other.position, other.size);
    }
};


struct GameState {
    Sprite background;
    Entity player;
    #define MAX_ENEMIES 8
    Entity enemies[MAX_ENEMIES]; 
    #define MAX_LASERS 15
    Entity lasers[MAX_LASERS];

    bool isGameOver = false;
    u32 score;
    GLTtext* scoreText;
    GLTtext* endGameText;

    Camera camera;

    static GameState& GetGameState() {
        static GameState gs = {};
        return gs;
    }
};

#endif