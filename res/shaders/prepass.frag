 

#include "globals.glsl"

void main() {
    float depth = gl_FragCoord.z;
    vec4 col = vec4(vs_in.fragNormal, depth);
    fragColor = col;
}