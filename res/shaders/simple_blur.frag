
#include "globals.glsl"

in vec2 fragTexCoord;
uniform sampler2D mainTex;
uniform int shouldFlipY;

void main()
{
    vec2 uv = fragTexCoord;
    uv.y = (shouldFlipY*(1-uv.y)) + ((1-shouldFlipY)*(uv.y));
    vec2 texelSize = vec2(GetTexelSize(mainTex));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(mainTex, uv + offset).r;
        }
    }
    fragColor = vec4(result) / (4.0 * 4.0);
}  
