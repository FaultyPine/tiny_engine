#include "statue.h"
#include "tiny_engine/tiny_fs.h"

static Sprite activatedStatueSprite;
static Sprite deactivatedStatueSprite;

void Statue::Initialize(glm::vec2 pos) {
    entity.name = "Statue";
    TextureProperties texProps = TextureProperties::Default();
    texProps.magFilter = TextureProperties::TexMagFilter::NEAREST;
    texProps.minFilter = TextureProperties::TexMinFilter::NEAREST;

    if (!activatedStatueSprite.isValid()) activatedStatueSprite = Sprite(LoadTexture(UseResPath("potp/statue.png"), texProps));
    if (!deactivatedStatueSprite.isValid()) deactivatedStatueSprite = Sprite(LoadTexture(UseResPath("potp/statue.png"), texProps));
    entity.sprite = deactivatedStatueSprite;
    f32 spriteWidth = entity.sprite.GetTextureWidth();
    f32 spriteHeight = entity.sprite.GetTextureHeight();
    f32 statueSpriteSize = 50.0;
    f32 statueAspectRatio = spriteHeight/spriteWidth;

    entity.size = glm::vec2(statueSpriteSize, statueSpriteSize*statueAspectRatio);
    entity.position = glm::vec3(pos.x - (entity.size.x/2.0), pos.y - (entity.size.y/2.0), 0.0);
}

void Statue::Toggle() {
    isActivated = !isActivated;
    entity.sprite = isActivated ? activatedStatueSprite : deactivatedStatueSprite;
}

void Statue::Draw() const {
    entity.Draw();
}