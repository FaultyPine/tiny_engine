#include "globals.glsl"
#include "common.glsl"

in vec2 fragTexCoord;
uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

uniform float palette_hue = 1.0;
uniform float palette_sat = 1.0;
uniform float palette_val = 1.0;
uniform float palette_brightness;
uniform float palette_darkness;
uniform float palette_tint;
uniform float palette_saturation;
uniform vec3 palette_base;
uniform float palette_scene_weight = 0.0;

vec3 Tonemap(vec3 col)
{
    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    const float gamma = 2.2;
    // reinhard tone mapping (simple HDR -> LDR)
    col = col / (col + vec3(1.0));
    // gamma correction 
    col = pow(col, vec3(1.0 / gamma));
    return col;
}

float posterize(float val, float levels)
{
    return round(val * levels) / levels;
}

vec3 palettize(vec3 col, vec3 base, float brightness, float darkness, float tint, float saturation)
{
    vec3 hsv = rgb2hsv(col);
    vec3 baseHsv = rgb2hsv(base);

    brightness = clamp(pow(hsv.b, 2.0 - brightness) + (darkness - 1.0), 0.0, 1.0);

    float hue = baseHsv.r + ((baseHsv.b - brightness) * tint);
    float sat = baseHsv.g + ((baseHsv.b - brightness) * saturation);

    float finalhue = clamp(posterize(hue, palette_hue), 0.0, 1.0);
    float finalsat = clamp(posterize(sat, palette_sat), 0.0, 1.0);
    float finalval = clamp(posterize(brightness, palette_val), 0.0, 1.0);
    vec3 result = vec3(finalhue, finalsat, finalval);
    result = hsv2rgb(result);
    return result;
}

void main()
{
    vec2 uv = fragTexCoord;
    float uvY = (shouldFlipY*(1-uv.y)) + ((1-shouldFlipY)*(uv.y));
    uv = vec2(uv.x, uvY);
    vec4 scene = texture(mainTex, uv);

    if (palette_scene_weight > 0.0)
    {
        vec3 palettizedScene = palettize(scene.rgb, palette_base, palette_brightness, palette_darkness, palette_tint, palette_saturation);
        scene.rgb *= palettizedScene * palette_scene_weight;
        //scene.rgb = palettizedScene;
    }

    // TODO: bloom

    scene.rgb = Tonemap(scene.rgb);
    fragColor = scene;
}  