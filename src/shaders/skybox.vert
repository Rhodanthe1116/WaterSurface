// https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/06%20Cubemaps/
#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 P;
uniform mat4 V;

void main()
{
    vec4 pos = P * V * vec4(position, 1.0);
    gl_Position = pos.xyww;
    TexCoords = position;
}