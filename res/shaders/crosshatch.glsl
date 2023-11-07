


float luma(vec3 color) {
    return (color.r + color.g + color.g) / 3;
    //return dot(color, vec3(0.299, 0.587, 0.114));
}
vec3 crosshatch(vec3 texColor, float x, float y, float z, float t1, float t2, float t3, float t4, float crosshatchOffset, float lineThickness) {
  float lum = luma(texColor);
  vec3 color = vec3(1.0);
  float crosshatchLineStep = crosshatchOffset / lineThickness;
  if (lum < t1) {
      float ch = mod(x + y + z, crosshatchOffset);
      ch = smoothstep(0,crosshatchLineStep, ch);
      color *= vec3(ch);
  }
  if (lum < t2) {
      float ch = mod(x - y - z, crosshatchOffset);
      ch = smoothstep(0,crosshatchLineStep, ch);
      color *= vec3(ch);
  }
  if (lum < t3) {
      float ch = mod(x + y + z - crosshatchOffset/2, crosshatchOffset);
      ch = smoothstep(0,crosshatchLineStep, ch);
      color *= vec3(ch);
  }
  if (lum < t4) {
      float ch = mod(x - y - z - crosshatchOffset/2, crosshatchOffset);
      ch = smoothstep(0,crosshatchLineStep, ch);
      color *= vec3(ch);
  }
  return color;
}

vec3 crosshatch(vec3 texColor, vec3 fragPosWs) {
  float crosshatchOffset = 0.8;
  float lineThickness = 8;
  //vec2 xy = vec2(gl_FragCoord.x, gl_FragCoord.y);
  vec3 xyz = fragPosWs;
  return crosshatch(texColor, xyz.x, xyz.y, xyz.z, 1.0, 0.5, 0.2, 0.1, crosshatchOffset, lineThickness);
}