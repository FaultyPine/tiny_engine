#version 330 core

// Input vertex attributes (from vertex shader)
in vec3 fragPositionWS;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormalOS;
in vec4 fragPosLightSpace;
flat in int materialId;
in float waveHeight;
// Output fragment color
out vec4 finalColor;

uniform float nearClip;
uniform float farClip;

const vec3 waterColor = vec3(0.32,0.65,0.97);
const vec3 waterHighlight = vec3(0.7,1,1);
const vec3 foamColor = vec3(0.4, 1.0, 1.0);
const float offset = 0.4;
const float contrast = 3.1;
const float brightness = 4.0;

uniform sampler2D waterTexture;
uniform float time;
uniform vec2 waterfallPosWS = vec2(-0.5, 5.5);
uniform float waterfallRippleDecayFactor = 5.0;

vec3 Waterfall(vec3 ogCol) {
    float dist = distance(waterfallPosWS, fragPositionWS.xz);
    float mask = fract(dist-time);
    float distFalloff = pow(dist, waterfallRippleDecayFactor);
    mask *= distFalloff;
    mask = step(mask, 0.9);
    return mix(ogCol,foamColor, mask);
}

void main() {
    float time = time * 0.01;
    vec3 waterTextureCol = texture(waterTexture, fragTexCoord+time).rgb;
    float alpha = length(waterTextureCol)*1.2;
    
    float mask = (waveHeight - offset) * contrast;
    vec3 col = mix(waterColor, waterHighlight, mask) * brightness;

    col *= waterTextureCol;
    col = Waterfall(col);

    col = pow(col, vec3(1.0/2.2)); // gamma correction
    finalColor = vec4(vec3(col), alpha);
}