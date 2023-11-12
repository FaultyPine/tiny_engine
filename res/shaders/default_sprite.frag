 

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D mainTex;
uniform vec4 color;
// 0 for no flip, 1 for flip
uniform int shouldFlipY;

void main()
{
    float uvY = (shouldFlipY*(1-TexCoords.y)) + ((1-shouldFlipY)*(TexCoords.y));
    vec2 uv = vec2(TexCoords.x, uvY);
    //vec2 uv = TexCoords;
    FragColor = color * texture(mainTex, uv);
    if (FragColor.a < 0.0001) {
        discard;
    }
}  