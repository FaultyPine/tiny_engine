 

out vec4 outColor;
void main() {
    // this happens implicitly, explicitly putting this here for clarity
    gl_FragDepth = gl_FragCoord.z;
    outColor = vec4(vec3(gl_FragDepth), 1.0); // in case we ever want to use this for displaying the depth
}