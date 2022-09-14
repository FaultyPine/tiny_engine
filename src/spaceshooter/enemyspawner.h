#ifndef SPACESHOOTER_ENEMYSPAWNER_H
#define SPACESHOOTER_ENEMYSPAWNER_H

#include "spaceshooter_fwd.h"

namespace EnemySpawner {

const u32 ENEMY_SPAWN_RATE_SEC = 2;

void tickEnemySpawner(Entity* enemies, u32 numMaxEnemies) {
    u32 frame = GetFrameCount();

    // spawn enemy
    if (frame % 60*ENEMY_SPAWN_RATE_SEC == 0) {
        static u32 enemyIdx = 0;
        Entity& enemy = enemies[enemyIdx];
        if (!enemy.active) {
            // use object pool
            enemyIdx = (enemyIdx+1) % numMaxEnemies;

            enemy.active = true;
            u32 randX = rand() % (Camera::GetMainCamera().screenWidth - (u32)enemy.size.x);
            glm::vec2 randPos = glm::vec2(randX, 0.0f);
            enemy.position = glm::vec3(randPos, 0.0f);
        }
    }

}




}

#endif