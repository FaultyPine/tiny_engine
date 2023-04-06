#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 vertexTexCoord;
layout (location = 3) in vec4 vertexColor;
layout (location = 4) in int  vertexMaterialId;
layout (location = 5) in mat4 instanceModelMat;

// Input uniform values
uniform int numInstances;
uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform float time;
uniform mat4 lightSpaceMatrix;

// Output vertex attributes (to fragment shader)

out vec3 fragPositionWS;
out vec2 fragTexCoord;
//out vec4 fragColor;
out vec3 fragNormalOS;
//out vec4 fragPosLightSpace;
out vec4 fragPosLightSpace;
flat out int materialId;
//out vec3 fragPositionOS;

float cnoise(vec2 P);

vec2 windDirBase = vec2(0.5, 0.5);
uniform sampler2D windTexture;
uniform float _WindStrength;
uniform float _WindFrequency;
uniform float _WindUVScale;

void GetGrassSway(inout vec3 vertPosOS) {
    float height = pow(vertexTexCoord.y, 4);
    vec3 posWS = (instanceModelMat*vec4(vertPosOS, 1.0)).xyz;
    vec2 windDir = normalize( windDirBase + (cnoise(vec2(time))+1)/2 );

    vec2 uv = posWS.xz + (_WindFrequency * time);
    vec2 windSample = (texture(windTexture, uv/_WindUVScale).xy * 2 -1);
    vec2 wind = normalize(windSample) * _WindStrength;
    vertPosOS.xz += wind * height;
}
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

void main()
{
    mat4 modelView = viewMat * instanceModelMat;
    modelView = Billboard(modelView);
    mat4 mvp = projectionMat * modelView;
    
    vec3 vertPos = vertexPosition;
    GetGrassSway(vertPos);

    fragPositionWS = vec3(instanceModelMat*vec4(vertPos, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPositionWS, 1.0);
    fragNormalOS = vertexNormal;

    fragTexCoord = vertexTexCoord;
    materialId = vertexMaterialId;

    gl_Position = mvp*vec4(vertPos, 1.0);
}








//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
//
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}

float cnoise(vec2 P){
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;
  vec4 i = permute(permute(ix) + iy);
  vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
  vec4 gy = abs(gx) - 0.5;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);
  vec4 norm = 1.79284291400159 - 0.85373472095314 * 
    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}