#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalWS;
in vec4 fragPosLightSpace;
flat in int materialId;
in float waveHeight;
// Output fragment color
out vec4 finalColor;

uniform float nearClip;
uniform float farClip;

const vec3 waterColor = vec3(0.32,0.65,0.97);
const vec3 waterHighlight = vec3(0.7,1,1);
const float offset = 0.4;
const float contrast = 3.1;
const float brightness = 1.0;

uniform sampler2D waterTexture;
uniform float time;

void main() {
    float time = time * 0.01;
    vec3 waterTextureCol = texture(waterTexture, fragTexCoord+time).rgb;
    //vec3 fragthing = normalize( cross(dFdx(fragPositionWS), dFdy(fragPositionWS)) );
    
    float mask = (waveHeight - offset) * contrast;
    vec3 col = mix(waterColor, waterHighlight, mask) * brightness;
    col *= waterTextureCol;
    col = pow(col, vec3(1.0/2.2)); // gamma correction
    finalColor = vec4(vec3(col), length(waterTextureCol)*1.2);

}