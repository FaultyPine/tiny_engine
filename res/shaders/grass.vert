
#include "globals.glsl"

#include "common.glsl"
#include "noise.glsl"
#include "hash.glsl"


vec2 windDirBase = normalize(vec2(0.5, 0.5));
uniform sampler2D windTexture;
uniform float _WindStrength;
uniform float _WindFrequency;
uniform float _WindUVScale;
uniform float _CurveIntensity;

mat4 Billboard(mat4 modelViewMat) {
    // To create a "billboard" effect,
    // set upper 3x3 submatrix of model-view matrix to identity
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            modelViewMat[i][j] = 0;
        }
    }
    modelViewMat[0][0] = 1;
    modelViewMat[1][1] = 1;
    modelViewMat[2][2] = 1;
    return modelViewMat;
}


vec3 positionFromModelMat(mat4 model)
{
    return vec3(model[3]);
}

float scaleFromModelMat(mat4 model)
{
    return sqrt(model[0][0] * model[0][0] + model[1][1] * model[1][1] + model[2][2] * model[2][2]);
}

vec3 GrassWind(vec3 vertexPosition)
{
    vec3 vertPos = vertexPosition;
    // NOTE: doing all calculations in world space to account for grass blades
    // having randomized local rotation
    vec3 ogVertPositionWS = vec3(instanceModelMat*vec4(vertPos, 1.0));

    // TODO: rotate grass blades slightly more towards camera in view space
    // https://www.youtube.com/watch?v=bp7REZBV4P4&ab_channel=SimonDev

    // new grass
    vec3 grassPosition = positionFromModelMat(instanceModelMat);
    float noise = cnoise01(grassPosition.xz * _WindUVScale + time * _WindFrequency);
   
    // sample noise from world pos for wind *direction* and remap [0,1] to some angle range
    float windDirFactor = noise;
    float windDirRadians = remap(windDirFactor, 0.0, 1.0, 0.0, 1 * PI);
    vec2 windDir2 = AngleToDir(windDirRadians);
    vec3 windDir = vec3(windDir2.x, 0.0, windDir2.y);
    // strength of the wind
    float windStrength = noise * _WindStrength;
    windDir.xz *= windStrength;

    ogVertPositionWS -= grassPosition;
    // add wind to curve of grass
    float grassSway = noise;
    float curveAmount = _CurveIntensity * grassSway;
    ogVertPositionWS = vec3(vec4(ogVertPositionWS, 1.0) * rotation(windDir, curveAmount * vertexTexCoord.y));
    ogVertPositionWS += grassPosition;


    // making sure we're using our wind direction in world space
    // since the grass blades have random local rotation
    ogVertPositionWS += (windDir) * vertexTexCoord.y;
    vertPos = vec3(inverse(instanceModelMat) * vec4(ogVertPositionWS, 1.0));
    return vertPos;
}

void main()
{
    VertexToFrag();
    mat4 modelView = GetViewMatrix() * instanceModelMat;
    //modelView = Billboard(modelView);
    mat4 mvp = GetProjectionMatrix() * modelView;
    
    vec3 vertPos = vertexPosition;
    vertPos = GrassWind(vertPos) * scaleFromModelMat(instanceModelMat);

    //float displacementAmount = length(vertPos - GrassWind(vertPos+0.1));

    vs_out.fragPositionWS = vec3(instanceModelMat*vec4(vertPos, 1.0));
    vec3 tangent = normalize(vertPos - GrassWind(vertPos+0.1));
    vec3 normal = cross(tangent, vec3(0,1,0));
    //normal = mix(normal, vec3(0,1,0), displacementAmount);
    //vec3 normal = vec3(vec4(vertexNormal,1.0)*instanceModelMat);
    vs_out.fragNormal = vec3(GetNormalMatrix()*vertexNormal);
    vs_out.fragTangent = tangent;

    vs_out.fragTexCoord = vertexTexCoord;

    gl_Position = mvp*vec4(vertPos, 1.0);
}