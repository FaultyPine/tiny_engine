#ifndef SPACESHOOTER_ENEMY_H
#define SPACESHOOTER_ENEMY_H

#include "spaceshooter_fwd.h"

namespace Enemy {

const f32 ENEMY_SPEED = 120.0f;

void updateEnemy(Entity& enemy) {
    // eww mutable dingleton
    GameState& gs = GameState::GetGameState();
    
    // exactly what Chabane was talking about... 
    // my enemy class now is messing with global gamestate

    // if collision w/laser, make inactive & disappear
    for (Entity& laser : gs.lasers) {
        if (laser.active && enemy.isCollidingWith(laser)) {
            enemy.active = false;
            laser.active = false;
            gs.score++;
        }
    }

    // if collision w/player, end game
    const Entity& player = gs.player;
    if (enemy.isCollidingWith(player)) {
        gs.isGameOver = true;
    }

    // update pos
    enemy.position.y += ENEMY_SPEED * GetDeltaTime();

    if (enemy.position.y > Camera::GetMainCamera().screenHeight) {
        enemy.active = false;
    }
}





}

#endif