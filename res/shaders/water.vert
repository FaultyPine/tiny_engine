 
#include "globals.glsl"

// Input uniform values
uniform mat4 lightSpaceMatrix;

out vec4 fragPosLightSpace;
out float waveHeight;

struct Wave {
    float waveSpeed;
    float wavelength;
    float steepness;
    vec2 direction;
};
#define NUM_WAVES 8
uniform Wave waves[NUM_WAVES];
uniform int numActiveWaves = 0;
#include "shader_defines.glsl"


vec3 GertsnerWave(vec3 VertexPos, in Wave waveOpts, inout vec3 tangent, inout vec3 binormal) {
    float wavelength = waveOpts.wavelength;
    float steepness = waveOpts.steepness;
    vec2 direction = normalize(waveOpts.direction);
    float waveTime = time * waveOpts.waveSpeed; 

    float waveNumber = 2 * PI / wavelength;
    float phaseSpeed = sqrt(9.8 / waveNumber);
    float waveDeriv = waveNumber * (dot(direction, VertexPos.xz) - phaseSpeed * waveTime);
    float amplitude = steepness / waveNumber;

    vec3 gertsnerWave = vec3(
        direction.x * (amplitude * cos(waveDeriv)),
        amplitude * sin(waveDeriv),
        direction.y * (amplitude * cos(waveDeriv))
    );

    tangent += vec3(
        -direction.x * direction.x * (steepness * sin(waveDeriv)),
        direction.x * (steepness * cos(waveDeriv)),
        -direction.x * direction.y * (steepness * sin(waveDeriv))
    );
    binormal += vec3(
        -direction.x * direction.y * (steepness * sin(waveDeriv)),
        direction.y * (steepness * cos(waveDeriv)),
        -direction.y * direction.y * (steepness * sin(waveDeriv))
    );

    return gertsnerWave;
}
vec3 GertsnerWaves(vec3 VertexPos, inout vec3 normal, inout float waveHeightOffset) {
    vec3 newVertexPos = VertexPos;
    vec3 tangent = vec3(0);
    vec3 binormal = vec3(0);
    for (int i = 0; i < numActiveWaves; i++) {
        vec3 gertsnerIteration = GertsnerWave(VertexPos, waves[i], tangent, binormal); // range [-1,1]
        newVertexPos += gertsnerIteration;
        waveHeightOffset += gertsnerIteration.y;
    }
    normal = normalize(cross(binormal, tangent));
    return newVertexPos;
}


void main() {
    VertexToFrag();
    vec3 vertexPosWS = vs_out.fragPositionWS;

    vec3 vertPos = vertexPosWS;
    vec3 normal = vec3(0);
    float wvh = 0.0;
    vec3 newVertexPos = GertsnerWaves(vertexPosWS, normal, wvh);
    waveHeight = wvh;
    vertPos = newVertexPos;

    gl_Position = GetProjectionMatrix() * GetViewMatrix() * GetModelMatrix() *vec4(vertPos, 1.0);
}