
#include "globals.glsl"

uniform sampler2D waterfallTex;

const vec3 waterColor = vec3(0.32,0.65,0.97);
const vec3 waterHighlight = vec3(0.7,1,1);
const float offset = 0.4;
const float contrast = 3.1;
const float brightness = 3.0;
const float colorBanding = 5.0;

void main() {
    vec2 texCoord = vs_in.fragTexCoord;
    float scroll = time * 0.4;
    vec2 uv = vec2(texCoord.x * 10, (texCoord.y-scroll));
    vec3 water = texture(waterfallTex, uv).rgb * 0.7;
    water = round(water * colorBanding) / colorBanding;

    float mask = (water.r - offset) * contrast;
    vec3 col = mix(waterColor, waterHighlight, mask) * brightness;
    col *= water;
    
    fragColor = vec4(col, 1.0);
}