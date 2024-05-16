
#include "shader_defines.glsl"

float clamp01(float x)
{
    return clamp(x, 0.0, 1.0);
}

vec2 NormalToDir(vec2 dir)
{
    return vec2(-dir.y, dir.x);
}

vec3 NormalToDir(vec3 dir, vec3 up)
{
    return cross(dir, up);
}

mat3 rotateX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat3(1,  0, 0,
                0,  c, -s,
                0,  s, c);
}

mat3 rotateY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat3(c,  0,  s,
                0,  1,  0,
                -s, 0,  c);
}

mat3 rotateZ(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat3(c, -s, 0,
                s, c,  0,
                0, 0,  1);
}

mat4 rotation(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec2 AngleToDir(float angleRadians)
{
    return normalize(vec2(cos(angleRadians), sin(angleRadians)));
}

float remap(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

// we go from ndc -> clip space -> projection -> view space -> world space
vec3 PositionFromDepth(mat4 viewProjectionMatrix, vec2 uv, float depth) {
    float far = farClip;
    float near = nearClip;
    vec4 clipSpacePosition = vec4(uv * 2 - 1, (2 * depth - near - far) / (far - near), 1);
    vec4 position = inverse(viewProjectionMatrix) * clipSpacePosition;
    position /= position.w;
    return position.xyz;
}

// All components are in the range [0…1], including hue.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0…1], including hue.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

