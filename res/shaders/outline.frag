 
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D mainTex;
uniform sampler2D depthNormals;
uniform vec3 viewDir;
uniform float screenWidth;
uniform float screenHeight;
uniform float time;

uniform float _OutlineScale;

uniform float _DepthThreshold;
uniform float _DepthThickness;
uniform float _DepthStrength = 1.0;
//uniform float _AcuteAngleDepthDot = 1.0;


uniform float _ColorThreshold;
uniform float _ColorThickness;
uniform float _ColorStrength = 1.0;

uniform float _NormalThreshold;
uniform float _NormalThickness;
uniform float _NormalStrength = 1.0;

#include "crosshatch.glsl"

// https://alexanderameye.github.io/notes/rendering-outlines/
// https://alexanderameye.github.io/notes/edge-detection-outlines/

float linearize_depth(float d,float zNear,float zFar) {
    //return zNear * zFar / (zFar + d * (zNear - zFar));
    //return (2.0 * zNear * zFar) / (zFar + zNear - (d * 2.0 - 1.0) * (zFar - zNear));
    float z = d * 2.0 - 1.0; // back to NDC 
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

float SampleDepth(vec2 uv) {
    // depth is stored in alpha channel of depthNormals texture
    float depth = texture(depthNormals, uv).a;
    return depth;
}
float SampleDepthLin(vec2 uv) {
    return linearize_depth(SampleDepth(uv), 0.01, 100.0);
}
vec3 SampleNormals(vec2 uv) {
    vec3 normsSample = texture(depthNormals, uv).rgb;
    return normsSample;
}

vec2 sobelSamplePoints[9] = vec2[](
    vec2(-1,1), vec2(0,1), vec2(1,1),
    vec2(-1,0), vec2(0,0), vec2(1,1),
    vec2(-1,-1), vec2(0,-1), vec2(1,-1)
);
float sobelXMatrix[9] = float[](
    1, 0, -1,
    2, 0, -2,
    1, 0, -1
);
// TODO: can i just take the transpose of the x mat to get y?
float sobelYMatrix[9] = float[](
    1, 2, 1,
    0, 0, 0,
    -1, -2, -1
);

float DepthSobel(vec2 uv, float thickness) {
    vec2 sobel = vec2(0);
    for (int i = 0; i < 9; i++) {
        float depth = SampleDepth(uv + sobelSamplePoints[i] * thickness);
        vec2 diff = depth * vec2(sobelXMatrix[i], sobelYMatrix[i]);
        sobel += diff;
    }
    return length(sobel);
}
float NormalSobel(vec2 uv, float thickness) {
    vec2 sobelR = vec2(0);
    vec2 sobelG = vec2(0);
    vec2 sobelB = vec2(0);

    for (int i = 0; i < 9; i++) {
        vec3 sceneCol = SampleNormals(uv + sobelSamplePoints[i] * thickness);
        vec2 kernel = vec2(sobelXMatrix[i], sobelYMatrix[i]);

        sobelR += sceneCol.r * kernel;
        sobelG += sceneCol.g * kernel;
        sobelB += sceneCol.b * kernel;
    }
    // combine rgb by taking largest sobel
    //float ret = max(length(sobelR), max(length(sobelG), length(sobelB)));
    // combine by taking average
    float ret = (length(sobelR) + length(sobelG) + length(sobelB)) / 3.0;
    return ret;
}
float ColorSobel(vec2 uv, float thickness) {
    vec2 sobelR = vec2(0);
    vec2 sobelG = vec2(0);
    vec2 sobelB = vec2(0);

    for (int i = 0; i < 9; i++) {
        vec3 sceneCol = texture(mainTex, uv + sobelSamplePoints[i] * thickness).rgb;
        vec2 kernel = vec2(sobelXMatrix[i], sobelYMatrix[i]);

        sobelR += sceneCol.r * kernel;
        sobelG += sceneCol.g * kernel;
        sobelB += sceneCol.b * kernel;
    }
    // combine rgb by taking largest sobel
    //float ret = max(length(sobelR), max(length(sobelG), length(sobelB)));
    // combine by taking average
    float ret = (length(sobelR) + length(sobelG) + length(sobelB)) / 3.0;
    return ret;
}

float FineTuneDepthSobel(float sobel, float depthThreshold) {
    sobel = smoothstep(0, depthThreshold, sobel);
    sobel = pow(sobel, _DepthThickness);
    sobel *= _DepthStrength;
    return sobel;
}
float FineTuneColorSobel(float sobel, float colorThreshold) {
    sobel = smoothstep(0, colorThreshold, sobel);
    sobel = pow(sobel, _ColorThickness);
    sobel *= _ColorStrength;
    return sobel;
}
float FineTuneNormalSobel(float sobel, float normalThreshold) {
    sobel = smoothstep(0, normalThreshold, sobel);
    sobel = pow(sobel, _NormalThickness);
    sobel *= _NormalStrength;
    return sobel;
}

float Sobel(vec2 uv) {
    float sobel = 0.0;

    // TODO: (halfway done here) account for viewing surfaces at a very steep angle
    //float viewDotNorm = 1.0 - dot(viewDir, SampleNormals(uv));
    //float perpindicularAmt = smoothstep(_AcuteAngleDepthDot, 1, viewDotNorm);
    //float depthThreshold = mix(0, _DepthThreshold, perpindicularAmt);

    // TODO: handle far away outlines by raising threshold for objects far away from cam
    float depth = SampleDepth(uv);

    float depthSobel = DepthSobel(uv, _DepthThickness);
    depthSobel = FineTuneDepthSobel(depthSobel, _DepthThreshold*depth);

    float colorSobel = ColorSobel(uv, _ColorThickness);
    colorSobel = FineTuneColorSobel(colorSobel, _ColorThreshold*depth);

    float normalSobel = NormalSobel(uv, _NormalThickness);
    normalSobel = FineTuneNormalSobel(normalSobel, _NormalThreshold*depth);

    sobel = max(normalSobel, max(depthSobel, colorSobel));

    return sobel;
}

void main()
{
    //vec2 texelSize = 1.0 / textureSize(mainTex, 0); // gets size of single texel
    vec2 uv = vec2(TexCoords.x, 1-TexCoords.y);
    vec3 screenTex = texture(mainTex, uv).rgb;
    vec3 col = screenTex;

    float sobel = Sobel(uv);

    //sobel *= 16;
    //col = mix(col, vec3(0.0), sobel);
    col = vec3(1-sobel);

    FragColor = vec4(col, 1.0);
}








