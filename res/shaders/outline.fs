#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D mainTex;
uniform sampler2D depthMap;
uniform float screenWidth;
uniform float screenHeight;
uniform float time;

uniform float _OutlineScale;
uniform float _DepthThreshold;

void Outline(inout vec3 col, vec2 texelSize, vec2 texcoords) {
    float halfScaleFloor = floor(_OutlineScale * 0.5);
    float halfScaleCeil = ceil(_OutlineScale * 0.5);

    vec2 bottomLeftUV = texcoords - vec2(texelSize.x, texelSize.y) * halfScaleFloor;
    vec2 topRightUV = texcoords + vec2(texelSize.x, texelSize.y) * halfScaleCeil;  
    vec2 bottomRightUV = texcoords + vec2(texelSize.x * halfScaleCeil, -texelSize.y * halfScaleFloor);
    vec2 topLeftUV = texcoords + vec2(-texelSize.x * halfScaleFloor, texelSize.y * halfScaleCeil);

    float depth0 = texture(depthMap, bottomLeftUV).r;
    float depth1 = texture(depthMap, topRightUV).r;
    float depth2 = texture(depthMap, bottomRightUV).r;
    float depth3 = texture(depthMap, topLeftUV).r;
    float depthThreshold = _DepthThreshold * depth0;

    float depthFiniteDifference0 = depth1 - depth0;
    float depthFiniteDifference1 = depth3 - depth2;

    float edgeDepth = sqrt(pow(depthFiniteDifference0, 2) + pow(depthFiniteDifference1, 2)) * 100;
    edgeDepth = step(_DepthThreshold, edgeDepth);

    col -= edgeDepth;
}

void main()
{
    vec2 texelSize = 1.0 / textureSize(mainTex, 0); // gets size of single texel
    vec2 uv = vec2(TexCoords.x, 1-TexCoords.y);
    vec3 screenTex = texture(mainTex, uv).rgb;
    vec3 depth = texture(depthMap, uv).rgb;
    vec3 col = screenTex;
    
    FragColor = vec4(depth, 1.0);
}