 
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D mainTex;
uniform bool shouldFlipY;

vec2 center = vec2(0.5, 0.5);

void main()
{
    vec2 uv = vec2(TexCoords.x, shouldFlipY ? 1.0-TexCoords.y : TexCoords.y);
    //vec2 uv = TexCoords;
    vec3 screenTex = texture(mainTex, uv).rgb;
    
    vec3 col = screenTex;
    FragColor = vec4(col, 1.0);
}