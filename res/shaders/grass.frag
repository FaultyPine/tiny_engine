#version 330 core

// Input vertex attributes (from vertex shader)

in vec3 fragPositionWS;
in vec2 fragTexCoord;
//in vec4 fragColor;
//in vec3 fragNormalWS;
in vec4 fragPosLightSpace;
flat in int materialId;
//in vec3 fragPositionOS;
// Output fragment color
out vec4 finalColor;
uniform int numInstances;

vec3 crosshatch(vec3 texColor);

uniform sampler2D shadowMap;
float PCFShadow(vec2 projCoords, float shadowBias, float currentDepth, int resolution) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -resolution; x <= resolution; x++) {
        for(int y = -resolution; y <= resolution; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= float(pow(resolution*2 + 1, 2));
    return shadow;
}

// 0 is in shadow, 1 is out of shadow
float GetShadow() {
    const float shadowBias = 0.005;
    // maximum bias of 0.05 and a minimum of 0.005 based on the surface's normal and light direction
    //float shadowBias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);  
    // manual perspective divide
    // range [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) // if fragment in light space is outside the frustum, it should be fully lit
        return 1.0;


    // depth value from shadow map
    float shadowMapDepth = texture(shadowMap, projCoords.xy).r;
    // [0,1] current depth of this fragment
    float currentDepth = projCoords.z;
    // 1.0 is in shadow, 0 is out of shadow

    //float shadow = PCFShadow(projCoords.xy, shadowBias, currentDepth, 1);

    // - bias   gets rid of shadow acne
    float shadow = currentDepth-shadowBias > shadowMapDepth ? 1.0 : 0.0;
    
    return 1-shadow;
}

const vec3 brown = vec3(115, 72, 69);


void main() {
    float height = fragTexCoord.y;
    vec3 brown = brown/255.0;
    vec3 col = mix(vec3(0,0.5,0), brown, 1-height);

    // don't want shadows making it completely black
    // this kinda simulates ambient light
    float shadow = max(GetShadow(), 0.4); 
    //shadow = crosshatch(vec3(shadow)).r;
    //col *= shadow;

    finalColor = vec4(vec3(col), 1.0);
}









float luma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}
vec3 crosshatch(vec3 texColor, float x, float y, float t1, float t2, float t3, float t4, float crosshatchOffset, float lineThickness) {
  float lum = luma(texColor);
  vec3 color = vec3(1.0);
  float crosshatchLineStep = crosshatchOffset / lineThickness;
  if (lum < t1) {
      float ch = mod(x + y, crosshatchOffset);
      ch = step(crosshatchLineStep, ch);
      color = vec3(ch);
  }
  if (lum < t2) {
      float ch = mod(x - y, crosshatchOffset);
      ch = step(crosshatchLineStep, ch);
      color = vec3(ch);
  }
  if (lum < t3) {
      float ch = mod(x + y - crosshatchOffset/2, crosshatchOffset);
      ch = step(crosshatchLineStep, ch);
      color = vec3(ch);
  }
  if (lum < t4) {
      float ch = mod(x - y - crosshatchOffset/2, crosshatchOffset);
      ch = step(crosshatchLineStep, ch);
      color = vec3(ch);
  }
  return color;
}

vec3 crosshatch(vec3 texColor) {
  float crosshatchOffset = 0.1;
  float lineThickness = 4;
  //vec2 xy = vec2(gl_FragCoord.x, gl_FragCoord.y);
  vec2 xy = fragPositionWS.xy;
  return crosshatch(texColor, xy.x, xy.y, 0.5, 0.3, 0.1, 0.01, crosshatchOffset, lineThickness);
}