#version 430 core

// common input
layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal_modelspace;
layout (location = 2) in vec2 vertexUV;

// common
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 LightPosition_worldspace;
uniform vec3 EyePosition_worldspace;

// sinewave
uniform float 
amplitude,
wavelength,
time,
speed,
interactive_amplitude,
interactive_wavelength,
interactive_speed,
interactive_radius;

uniform vec2 drop_point;
uniform float drop_time;

// common output
out V_OUT
{
   vec3 Position_worldspace;
   vec3 EyeDirection_cameraspace;
   vec3 LightDirection_cameraspace;
   vec3 Normal_cameraspace;
   vec3 Normal_worldspace;
   vec2 UV;
} v_out;



void commonOutput(
    vec3 calculated_vertexPosition_modelspace,
    vec3 calculated_vertexNormal_modelspace
) {
    vec3 vertexPosition_modelspace = calculated_vertexPosition_modelspace;
    vec3 vertexNormal_modelspace = calculated_vertexNormal_modelspace;

    gl_Position = P * V * M * vec4(vertexPosition_modelspace, 1);
    v_out.Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;

    // Vector that goes from the vertex to the camera, in camera space.
    // In camera space, the camera is at the origin (0,0,0).
    vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
    v_out.EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
    vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
    v_out.LightDirection_cameraspace = LightPosition_cameraspace + v_out.EyeDirection_cameraspace;

    // Normal of the the vertex, in camera space
    v_out.Normal_cameraspace  = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;
    v_out.UV = vertexUV;

    v_out.Normal_worldspace  = (M * vec4(vertexNormal_modelspace,1)).xyz;

};


void main()
{
    float k = 2 * 3.14159 / wavelength;
    float f = k * (vertexPosition_modelspace.x - speed * time);
    vec3 new_sinewave_position_modelspace = vertexPosition_modelspace;
    float tmp_height = amplitude * sin(f);
    float tmp_interactive = 0.0f;
    if(drop_point.x >0.0f){
        float dist = distance(vertexUV, drop_point) / interactive_wavelength*100;
        float t_c = (time-drop_time)*(interactive_radius*3.1415926)*interactive_speed;
        tmp_interactive = interactive_amplitude * sin((dist-t_c)*clamp(0.0125*t_c,0,1))/(exp(0.1*abs(dist-t_c)+(0.05*t_c)))*1.5;
    }
     if((tmp_height <0 && tmp_interactive >0)||(tmp_height >0 && tmp_interactive <0)){
        new_sinewave_position_modelspace.y += (tmp_height + tmp_interactive);
    }
    else{
        new_sinewave_position_modelspace.y += (abs(tmp_height) > abs(tmp_interactive)) ? tmp_height:tmp_interactive;
    }
    vec3 tangent = normalize(vec3(1,k*amplitude*cos(f),0));



    commonOutput(
        new_sinewave_position_modelspace, 
        vec3(-tangent.y, tangent.x, 0)
    );
};
