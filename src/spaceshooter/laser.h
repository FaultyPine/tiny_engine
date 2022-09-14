#ifndef SPACESHOOTER_LASER_H
#define SPACESHOOTER_LASER_H

#include "spaceshooter_fwd.h"

namespace Laser {

const f32 LASER_SPEED = 245.0f;


bool attemptSpawnLaser(const glm::vec2& position) {
    GameState& gs = GameState::GetGameState();

    static u32 laserIdx = 0;
    Entity& laser = gs.lasers[laserIdx];
    if (laser.active) {
        // if the laser we are trying to spawn is already active, 
        // we've reached the limit of the object pool. Don't spawn laser
        return false;
    }
    // use object pool
    laserIdx = (laserIdx+1) % MAX_LASERS;

    laser.active = true;
    laser.position = glm::vec3(position, 1.0f);

    return true;
}

void updateLaser(Entity& laser) {
    // update position
    laser.position.y -= LASER_SPEED  * GetDeltaTime();
    // deactivate if off (top of) screen
    if (laser.position.y < 0) {
        laser.active = false;
    }
}

}

#endif