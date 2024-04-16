#include "globals.glsl"

in vec2 fragTexCoord;
uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

void main()
{
    vec2 uv = fragTexCoord;
    float uvY = (shouldFlipY*(1-uv.y)) + ((1-shouldFlipY)*(uv.y));
    uv = vec2(uv.x, uvY);
    vec4 scene = texture(mainTex, uv);
    fragColor = scene;
}  