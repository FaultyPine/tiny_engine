#ifndef POTP_FWD
#define POTP_FWD

#include "tiny_engine/pch.h"
#include "tiny_engine/sprite.h"
#include "tiny_engine/math.h"

#define MAX_NUM_PLAYERS 4

// Technically ... can't match zero macro arguments, so MAKE_FUNCTION_TYPE(myFunc, int) is officially invalid. But most compilers allow it anyway
#define MAKE_FUNCTION_TYPE(name, ret, ...) typedef ret (*name)(__VA_ARGS__)

struct GameState;
struct UserInput;
MAKE_FUNCTION_TYPE(SceneInit, void, GameState&);
MAKE_FUNCTION_TYPE(SceneTick, void, GameState&, UserInput&);
MAKE_FUNCTION_TYPE(SceneDraw, void, const GameState&, const UserInput&);
MAKE_FUNCTION_TYPE(SceneEnd, void, GameState&);

#define NO_IMPL_PRINT_() std::cout << "No implementation!" << __LINE__ << "\n"
inline void NoSceneInit(GameState& gs) {NO_IMPL_PRINT_();}
inline void NoSceneTick(GameState& gs, UserInput& inputs) {NO_IMPL_PRINT_();}
inline void NoSceneDraw(const GameState& gs, const UserInput& inputs) {NO_IMPL_PRINT_();}
inline void NoSceneEnd(GameState& gs) {NO_IMPL_PRINT_();}

// for text rendering - allows text to scale as window resizes
inline f32 GetWindowWidthScaleFactor() {
    f32 width = Camera::GetScreenWidth();
    return width / Camera::GetMainCamera().GetMinScreenDimensions().x;
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
        sprite.DrawSprite(cam, position, size, rotation, glm::vec3(0.0, 0.0, 1.0), color);
    }
    inline bool isCollidingWith(const Entity& other) const {
        return Math::isOverlappingRect2D(this->position, this->size, other.position, other.size);
    }
};



#endif