
#include "globals.glsl"

in vec2 fragTexCoord;
uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

uniform sampler2D depthNormals;

float SSAO(vec2 uv, float depth, vec3 normal);

void main()
{
    vec2 uv = fragTexCoord;
    uv.y = (shouldFlipY*(1-uv.y)) + ((1-shouldFlipY)*(uv.y));
    vec4 depthAndNormals = texture(depthNormals, uv);
    float depth = depthAndNormals.a;
    vec3 normal = depthAndNormals.rgb;
    float ao = SSAO(uv, depth, normal);
    fragColor = vec4(ao.rrr, 1.0);
}  








#include "common.glsl"
// sync with postprocess.cpp
#define SSAO_KERNEL_SIZE 16
#define SSAO_NOISE_SIZE 16
uniform vec3 ssaoKernel[SSAO_KERNEL_SIZE];
uniform sampler2D ssaoNoise;
uniform float ssaoSampleRadius = 0.5;
uniform float occlusionPower = 1.0;

vec2 GetNoiseScale()
{
    float noiseSize = 4; // one "dimension" of the noise size (sqrt(SSAO_NOISE_SIZE))
    return vec2(screenSize.x / noiseSize, screenSize.y / noiseSize);
}


float SSAO(vec2 uv, float depth, vec3 normal)
{
    mat4 projMatrix = GetProjectionMatrix();
    mat4 viewMatrix = GetViewMatrix();
    vec2 screenUV = uv / screenSize.xy;
    vec3 worldPos = PositionFromDepth(projMatrix * viewMatrix, screenUV, depth, farClip, nearClip);
    vec3 posVS = vec3(viewMatrix * vec4(worldPos, 1));

    vec2 noiseUV = uv * GetNoiseScale();
    vec3 random = texture(ssaoNoise, noiseUV).rgb;
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float viewSpacePositionDepth = (viewMatrix * vec4(worldPos, 1)).z;

    float occlusion = 0.0;
    for (int i = 0; i < SSAO_KERNEL_SIZE; i++) 
    {
        vec3 samplePosition = TBN * ssaoKernel[i].xyz; // tangent to view space
        samplePosition = worldPos + samplePosition * ssaoSampleRadius;
        samplePosition = (viewMatrix * vec4(samplePosition, 1)).xyz;

        vec4 offset = projMatrix * vec4(samplePosition, 1);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        vec3 offsetPosition = PositionFromDepth(
            projMatrix * viewMatrix,
            offset.xy, depth, farClip, nearClip
        );
        float sampleDepth = (viewMatrix * vec4(offsetPosition, 1)).z;

        float rangeCheck = smoothstep(0.0, 1.0, ssaoSampleRadius / abs(viewSpacePositionDepth - sampleDepth));
        const float bias = 0.025;
        occlusion += (sampleDepth >= samplePosition.z + bias ? 1.0 : 0.0);  
    }
    occlusion = 1.0 - (occlusion / SSAO_KERNEL_SIZE);
    occlusion = pow(occlusion, occlusionPower);
    return occlusion;
}
