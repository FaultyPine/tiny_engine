 

#include "globals.glsl"
#include "common.glsl"

#define SSAO_KERNEL_SIZE 64
uniform vec3 ssaoKernel[SSAO_KERNEL_SIZE] = vec3[](
vec3(0.049771, -0.044709, 0.049963) ,
vec3(0.014575, 0.016531, 0.002239)  ,
vec3(-0.040648, -0.019375, 0.031934),
vec3(0.013778, -0.091582, 0.040924) ,
vec3(0.055989, 0.059792, 0.057659)  ,
vec3(0.092266, 0.044279, 0.015451)  ,
vec3(-0.002039, -0.054402, 0.066735),
vec3(-0.000331, -0.000187, 0.000369),
vec3(0.050045, -0.046650, 0.025385) ,
vec3(0.038128, 0.031402, 0.032868)  ,
vec3(-0.031883, 0.020459, 0.022515) ,
vec3(0.055702, -0.036974, 0.054492) ,
vec3(0.057372, -0.022540, 0.075542),
vec3(-0.016090, -0.003768, 0.055473),
vec3(-0.025033, -0.024829, 0.024951),
vec3(-0.033688, 0.021391, 0.025402),
vec3(-0.017530, 0.014386, 0.005348),
vec3(0.073359, 0.112052, 0.011015),
vec3(-0.044056, -0.090284, 0.083683),
vec3(-0.083277, -0.001683, 0.084987),
vec3(-0.010406, -0.032867, 0.019273),
vec3(0.003211, -0.004882, 0.004164),
vec3(-0.007383, -0.065835, 0.067398),
vec3(0.094141, -0.007998, 0.143350),
vec3(0.076833, 0.126968, 0.106999),
vec3(0.000393, 0.000450, 0.000302),
vec3(-0.104793, 0.065445, 0.101737),
vec3(-0.004452, -0.119638, 0.161901),
vec3(-0.074553, 0.034449, 0.224138),
vec3(-0.002758, 0.003078, 0.002923),
vec3(-0.108512, 0.142337, 0.166435),
vec3(0.046882, 0.103636, 0.059576),
vec3(0.134569, -0.022512, 0.130514),
vec3(-0.164490, -0.155644, 0.124540),
vec3(-0.187666, -0.208834, 0.057770),
vec3(-0.043722, 0.086925, 0.074797),
vec3(-0.002564, -0.002001, 0.004070),
vec3(-0.096696, -0.182259, 0.299487),
vec3(-0.225767, 0.316061, 0.089156),
vec3(-0.027505, 0.287187, 0.317177),
vec3(0.207216, -0.270839, 0.110132),
vec3(0.054902, 0.104345, 0.323106),
vec3(-0.130860, 0.119294, 0.280219),
vec3(0.154035, -0.065371, 0.229842),
vec3(0.052938, -0.227866, 0.148478),
vec3(-0.187305, -0.040225, 0.015926),
vec3(0.141843, 0.047163, 0.134847),
vec3(-0.044268, 0.055616, 0.055859),
vec3(-0.023583, -0.080970, 0.219130),
vec3(-0.142147, 0.198069, 0.005194),
vec3(0.158646, 0.230457, 0.043715),
vec3(0.030040, 0.381832, 0.163825),
vec3(0.083006, -0.309661, 0.067413),
vec3(0.226953, -0.235350, 0.193673),
vec3(0.381287, 0.332041, 0.529492),
vec3(-0.556272, 0.294715, 0.301101),
vec3(0.424490, 0.005647, 0.117578),
vec3(0.366500, 0.003588, 0.085702),
vec3(0.329018, 0.030898, 0.178504),
vec3(-0.082938, 0.512848, 0.056555),
vec3(0.867363, -0.002734, 0.100138),
vec3(0.455745, -0.772006, 0.003841),
vec3(0.417291, -0.154846, 0.462514),
vec3(-0.442722, -0.679282, 0.186503)
);

#define SSAO_NOISE_SIZE 16
uniform vec3 ssaoNoise[SSAO_NOISE_SIZE] = vec3[](
vec3(-0.909881, -0.480259, 0.000000),
vec3(0.320239, 0.600137, 0.000000),
vec3(0.499882, -0.137172, 0.000000),
vec3(-0.734008, 0.821295, 0.000000),
vec3(0.964721, -0.636306, 0.000000),
vec3(-0.809290, -0.472394, 0.000000),
vec3(-0.434653, -0.708922, 0.000000),
vec3(0.604223, -0.727863, 0.000000),
vec3(-0.844886, 0.738584, 0.000000),
vec3(0.254769, 0.159409, 0.000000),
vec3(-0.983812, 0.099720, 0.000000),
vec3(0.360574, -0.710090, 0.000000),
vec3(0.067866, 0.706062, 0.000000),
vec3(-0.122666, 0.244110, 0.000000),
vec3(-0.600898, -0.298095, 0.000000),
vec3(-0.723997, 0.026499, 0.000000)
);

#include "noise.glsl"
uniform float SSAO_SAMPLE_RADIUS = 0.5;
float SSAO(float depth, vec3 normal)
{
    vec2 screenUV = gl_FragCoord.xy / screenSize.xy;
    vec3 worldPos = PositionFromDepth(projection * view, screenUV, depth);

    int noiseidx = int(cnoise01(screenUV) * SSAO_NOISE_SIZE);
    vec3 random = ssaoNoise[noiseidx]; // TODO: pass in noise texture/uniforms instead of randomly using perlin here lol

    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float viewSpacePositionDepth = (view * vec4(worldPos, 1)).z;

    float occlusion = 0.0;
    for (int i = 0; i < SSAO_KERNEL_SIZE; i++) 
    {
        vec3 samplePosition = TBN * ssaoKernel[i].xyz;
        samplePosition = worldPos + samplePosition * SSAO_SAMPLE_RADIUS;
        samplePosition = (view * vec4(samplePosition, 1)).xyz;

        vec4 offset = projection * vec4(samplePosition, 1);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        vec3 offsetPosition = PositionFromDepth(
            projection * view,
            offset.xy, depth
        );
        float sampleDepth = (view * vec4(offsetPosition, 1)).z;

        float rangeCheck = smoothstep(0.0, 1.0, SSAO_SAMPLE_RADIUS / abs(viewSpacePositionDepth - sampleDepth));
        const float bias = 0.025;
        occlusion += (sampleDepth >= samplePosition.z + bias ? 1.0 : 0.0);  
    }

    return 1.0 - (occlusion / SSAO_KERNEL_SIZE);
}

layout (location = 1) out vec4 AOOutput;

void main() {
    float depth = gl_FragCoord.z;
    vec3 normal = vs_in.fragNormal;
    float ao = SSAO(depth, normal);
    vec4 col = vec4(normal, depth);
    fragColor = col; // 1st color attachment
    AOOutput = vec4(vec3(ao), 1.0); // 2nd color attachment
}