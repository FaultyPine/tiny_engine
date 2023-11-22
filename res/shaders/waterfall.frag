 

out vec4 finalColor;

in vec3 Normal;
in vec2 texCoord;

#include "globals.glsl"

uniform sampler2D waterfallTex;

const vec3 waterColor = vec3(0.32,0.65,0.97);
const vec3 waterHighlight = vec3(0.7,1,1);
const float offset = 0.4;
const float contrast = 3.1;
const float brightness = 3.0;
const float colorBanding = 5.0;

void main() {
    float scroll = time * 0.4;
    vec2 uv = vec2(texCoord.x * 10, (texCoord.y-scroll));
    vec3 water = texture(waterfallTex, uv).rgb * 0.7;
    water = round(water * colorBanding) / colorBanding;


    float mask = (water.r - offset) * contrast;
    vec3 col = mix(waterColor, waterHighlight, mask) * brightness;
    col *= water;

    
    //col = pow(col, vec3(1.0/2.2)); // gamma correction
    finalColor = vec4(col, 1.0);
}