#include "spritesheet.h"



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
            ASSERT(spriteY >= 0 && spriteY <= height);
            ASSERT(spriteX >= 0 && spriteX <= width);

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

void Spritesheet::SetAnimationIndices(u32 animKey, const std::vector<u32>& indices) {
    this->animationIndicesMap[animKey] = indices;
}
void Spritesheet::SetAnimation(u32 animKey, bool isLoop, s32 nonLoopNextAnim) {
    if (this->animation.animKey != animKey) {
        Animation anim;
        anim.animKey = animKey;
        anim.isLoop = isLoop;
        anim.frame = 0;
        anim.nonLoopNextAnim = nonLoopNextAnim;
        this->animation = anim;
    }
}
void Spritesheet::Tick() {
    ASSERT(this->animationIndicesMap.size() > 0 && "Spritesheet indices not set!");
    // if we should move to the next spritesheet frame
    if (this->framerateEnforcer % (60/this->framerate) == 0) {
        if (!this->animation.isLoop) {
            // if we're NOT looping, set animation to the "next anim" after this one is done
            this->animation.frame++;
            // if we're done with this anim
            if (this->animation.frame >= this->animationIndicesMap.at(this->animation.animKey).size()) {
                // IMPORTANT/TODO: Using the "next animation" will always set it to looping
                // you should be able to provide another animation which also has looping/next anim options
                this->SetAnimation(this->animation.nonLoopNextAnim);
            }
        }
        else {
            this->animation.frame = (this->animation.frame+1) % this->animationIndicesMap.at(this->animation.animKey).size();
        }
    }
    this->framerateEnforcer = (this->framerateEnforcer+1) % this->framerate;
}

void Spritesheet::Draw(const Camera& cam, glm::vec2 position, 
                glm::vec2 size, f32 rotate, glm::vec3 rotationAxis, glm::vec3 color) const {
    ASSERT(this->animationIndicesMap.size() > 0 && "Spritesheet indices not set!");
    const auto& indices = this->animationIndicesMap.at(this->animation.animKey);
    ASSERT(indices.size() > this->animation.frame);
    u32 spritesheetIdx = indices.at(this->animation.frame);
    this->sprites.at(spritesheetIdx).DrawSprite(cam, position, size, rotate, rotationAxis, color);
}