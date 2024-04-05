 

#include "globals.glsl"

void main() {
    float depth = gl_FragCoord.z;
    vec3 normal = vs_in.fragNormal;
    vec4 col = vec4(normal, depth);
    fragColor = col; // 1st color attachment
}