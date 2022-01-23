#version 330 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox_cube;

void main()
{
    color = texture(skybox_cube, TexCoords);
}