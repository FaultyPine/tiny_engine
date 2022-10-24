#ifndef TINY_SPRITESHEET_H
#define TINY_SPRITESHEET_H

#include "pch.h"
#include "sprite.h"
#include "texture.h"

// NOTE: there's a bit of animation logic in here... might be a good idea
// to seperate that out into an animation system
// gonna not do that for now for the sake of getting things done

struct Spritesheet {
    struct Animation {
        s32 animKey = -1;
        s32 frame = 0;
        s32 framerate = -1;
        bool isLoop = true;
        s32 nonLoopNextAnim = -1;
        Animation(s32 animKey) {
            this->animKey = animKey;
            frame = 0;
            framerate = -1;
            isLoop = true;
            nonLoopNextAnim = -1;
        }
        Animation() {
            Animation(-1);
        }
    };
    Spritesheet(){}
    Spritesheet(const char* spritesheetPath, u32 numRows, u32 numCols, TextureProperties props);

    Animation GetCurrentAnimation() { return animation; } 
    Sprite GetCurrentSprite() const;
    void SetAnimationIndices(s32 animKey, const std::vector<u32>& indices);
    void SetAnimation(Animation anim, bool forceOverride = false);
    void ResetAnim();
    void SetFrame(s32 frame);
    inline void SetDefaultFramerate(u32 defaultFramerate) { this->defaultFramerate = defaultFramerate; }
    void Tick();
    void Draw(const Camera& cam, glm::vec2 position, 
                glm::vec2 size, f32 rotate, glm::vec3 rotationAxis, glm::vec4 color) const;
private:
    std::vector<Sprite> sprites = {};
    // animation
    u32 framerateEnforcer = 0;
    u32 defaultFramerate = 60;
    Animation animation;
    // map of anim key -> list of indexes into the sprites list for the individual textures our "animation" should go through
    std::map<u32, std::vector<u32>> animationIndicesMap = {};
    
};

#endif