 
#include "globals.glsl"

out vec4 fragColor;

uniform vec4 color;
uniform int isHollow;


void main()
{
    float2 TexCoords = vs_in.fragTexCoord;
    if (isHollow == 1) {
        float edges = 0.95;
        float xright = step(edges, TexCoords.x);
        float ybottom = step(edges, TexCoords.y);
        float xleft = 1.0-step(1.0-edges, TexCoords.x);
        float ytop = 1.0-step(1.0-edges, TexCoords.y);
        float square = xright + xleft + ybottom + ytop;
        if (square <= 0) discard;
        fragColor = color;
    }
    else {
        fragColor = color;
    }
}  