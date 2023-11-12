 

out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 color;
uniform int isHollow;


void main()
{
    float halfway = 0.5; // halfway in texture space [0.0, 1.0]
    vec2 center = vec2(halfway, halfway);
    float distFromCenter = length(TexCoords - center);
    float circle = 1-step(halfway, distFromCenter);
    
    if (isHollow == 1) {
        float outlineThickness = 0.07; // higher = thicker
        float outline = step(halfway-outlineThickness, distFromCenter);
        circle *= outline;
    }

    if (circle == 0) discard;
    FragColor = color * circle;
}  