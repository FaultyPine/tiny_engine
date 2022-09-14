#ifndef SPACESHOOTER_PLAYER_H
#define SPACESHOOTER_PLAYER_H

#include "spaceshooter_fwd.h"
#include "laser.h"

namespace Player {



void updatePlayer(Entity& player, const UserInput& inputs) {
    #if 0
    // control with keyboard
    const f32 PLAYER_SPEED = 5.0f;
    if (inputs.isLeft()) {
        player.position.x -= PLAYER_SPEED;
    }
    if (inputs.isRight()) {
        player.position.x += PLAYER_SPEED;
    }
    #else
    // control with mouse
    f32 mouseX = inputs.GetMouse().mousePos.x;
    f32 maxXPos = Camera::GetMainCamera().screenWidth - player.size.x;
    player.position.x = CLAMP(mouseX, 0.0f, maxXPos);
    #endif

    // if player presses space, shoot laser
    static bool keyDown = false; // this sucks lol
    bool isKeyPressed = inputs.GetKeyDown(GLFW_KEY_SPACE) && !keyDown;
    keyDown = inputs.GetKeyDown(GLFW_KEY_SPACE);
    

    if (isKeyPressed) {
        glm::vec2 spawnPos = glm::vec2(player.position.x, player.position.y) + (player.size / 2.0f);
        bool didSpawnLaser = Laser::attemptSpawnLaser(spawnPos);
    }
}


}

#endif