#version 330 core


void main() {
    // this happens implicitly, explicitly putting this here for clarity
    gl_FragDepth = gl_FragCoord.z;
}