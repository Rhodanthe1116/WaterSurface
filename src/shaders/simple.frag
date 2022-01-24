#version 430 core
out vec4 f_color;

in V_OUT
{
   vec3 Position_worldspace;
   vec3 EyeDirection_cameraspace;
   vec3 LightDirection_cameraspace;
   vec3 Normal_cameraspace;
   vec3 normal;
   vec2 UV;
} f_in;

uniform sampler2D water;

void main()
{   
    vec3 color = vec3(texture(water, f_in.UV));
    f_color = vec4(color, 1.0f);
}