//#include "pch.h"
#include "spritesheet.h"
#include "tiny_engine.h"
#include "tiny_log.h"

Spritesheet::Spritesheet(const char* spritesheetPath, u32 numRows, u32 numCols, TextureProperties props) {
    // load spritesheet
    s32 width, height, numChannels;
    u8* imgData = LoadImageData(spritesheetPath, &width, &height, &numChannels);

    // determine how to iterate through the image
    s32 singleSpriteWidth = width / numCols;
    s32 singleSpriteHeight = height / numRows;

    // iterate through each sprite img
    for (s32 row = 0; row < numRows; row++) {
        for (s32 col = 0; col < numCols; col++) {
            // get sprite position
            f32 spriteY = row * singleSpriteHeight;
            f32 spriteX = col * singleSpriteWidth;
            TINY_ASSERT(spriteY >= 0 && spriteY <= height);
            TINY_ASSERT(spriteX >= 0 && spriteX <= width);

            SetPixelReadSettings(width, singleSpriteWidth*col, singleSpriteHeight*row);
            // NOTE: using glPixelStorei to tell opengl to skip pixels so that I can just use the x,y,width,height to
            // read the relevant part of the spritesheet
            Texture thisTex = GenTextureFromImg(imgData, singleSpriteWidth, singleSpriteHeight, props);
            
            s32 resultTexturesIdx = col + (row*numCols);
            this->sprites.emplace_back(Sprite(thisTex));            
        }
    }

    // back to defaults
    SetPixelReadSettings(0, 0, 0, 4);

    free(imgData);
}

void Spritesheet::SetAnimationIndices(s32 animKey, const std::vector<u32>& indices) {
    this->animationIndicesMap[animKey] = indices;
}
void Spritesheet::SetAnimation(Animation anim, bool forceOverride) {
    if (this->animation.animKey != anim.animKey || forceOverride) { // don't reset anim if we're requesting same one
        if (anim.framerate == -1) {
            anim.framerate = this->defaultFramerate;
        }
        this->animation = anim;
    }
}
void Spritesheet::ResetAnim() {
    this->animation.frame = 0;
}
void Spritesheet::SetFrame(s32 frame) {
    this->animation.frame = frame;
}
void Spritesheet::Tick() {
    TINY_ASSERT(this->animationIndicesMap.size() > 0 && "Spritesheet indices not set!");
    // if we should move to the next spritesheet frame
    if (this->animation.framerate > 0 && this->framerateEnforcer % (TARGET_FPS/this->animation.framerate) == 0) {
        if (!this->animation.isLoop) {
            // if we're NOT looping, set animation to the "next anim" after this one is done
            this->animation.frame++;
            // if we're done with this anim
            if (this->animation.frame >= this->animationIndicesMap.at(this->animation.animKey).size()) {
                // IMPORTANT/TODO: Using the "next animation" will always set it to looping
                // you should be able to provide another animation which also has looping/next anim options
                Animation nextAnim = {};
                nextAnim.animKey = this->animation.nonLoopNextAnim;
                this->SetAnimation(nextAnim);
            }
        }
        else {
            this->animation.frame = (this->animation.frame+1) % this->animationIndicesMap.at(this->animation.animKey).size();
        }
    }
    this->framerateEnforcer = (this->framerateEnforcer+1) % TARGET_FPS;
}

Sprite Spritesheet::GetCurrentSprite() const {
    TINY_ASSERT(this->animationIndicesMap.size() > 0 && "Spritesheet indices not set!");
    TINY_ASSERT(this->animation.animKey != -1);
    const auto& indices = this->animationIndicesMap.at(this->animation.animKey);
    TINY_ASSERT(indices.size() > this->animation.frame);
    u32 spritesheetIdx = indices.at(this->animation.frame);
    return this->sprites.at(spritesheetIdx);
}

void Spritesheet::Draw(glm::vec2 position, glm::vec2 size, f32 rotate, glm::vec3 rotationAxis, glm::vec4 color, bool adjustToScreensize) const {
    this->GetCurrentSprite().DrawSprite(position, size, rotate, rotationAxis, color, adjustToScreensize);
}