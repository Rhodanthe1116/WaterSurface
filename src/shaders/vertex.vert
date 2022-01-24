#version 430 core
// common input
layout (location = 0) in vec3 vertexPosition_modelspace;

out vec2 coord;

void main() {
    coord = vertexPosition_modelspace.xy * 0.5 + 0.5;
    //coord = vertexUV;
    // coord = vertexPosition_modelspace.xz * 0.5 + 0.5;
    gl_Position = vec4(vertexPosition_modelspace,  1.0);
}