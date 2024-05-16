#include "globals.glsl"

in vec2 fragTexCoord;
uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

uniform float exposure = 1.0;

void main()
{
    vec2 uv = fragTexCoord;
    float uvY = (shouldFlipY*(1-uv.y)) + ((1-shouldFlipY)*(uv.y));
    uv = vec2(uv.x, uvY);
    vec4 scene = texture(mainTex, uv);

    // TODO: bloom

    // Gamma correction   can also just glEnable(GL_FRAMEBUFFER_SRGB); before doing final mesh render
    const float gamma = 2.2;

    // reinhard tone mapping
    //scene.rgb = scene.rgb / (scene.rgb + vec3(1.0));
    // exposure tone mapping
    scene.rgb = vec3(1.0) - exp(-scene.rgb * exposure);
    // gamma correction 
    scene.rgb = pow(scene.rgb, vec3(1.0 / gamma));

    fragColor = scene;
}  