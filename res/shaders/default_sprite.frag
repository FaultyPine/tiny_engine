 

#include "globals.glsl"

in vec2 fragTexCoord;

uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

void main()
{
    vec2 TexCoords = fragTexCoord;
    float uvY = (shouldFlipY*(1-TexCoords.y)) + ((1-shouldFlipY)*(TexCoords.y));
    vec2 uv = vec2(TexCoords.x, uvY);
    //vec2 uv = TexCoords;
    fragColor = color * texture(mainTex, uv);
    if (fragColor.a < 0.0001) {
        discard;
    }
}  