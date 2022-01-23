// https://learnopengl-cn.readthedocs.io/zh/latest/04%20Advanced%20OpenGL/06%20Cubemaps/
#version 330 core
layout (location = 0) in vec3 position;
out vec3 TexCoords;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out V_OUT
{
   vec3 Position_worldspace;
} v_out;


void main()
{
    vec4 pos = P * V * M * vec4(position, 1.0);
    gl_Position = pos;
    TexCoords = position;
    v_out.Position_worldspace = position;
}