#version 430 core

// common input
layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal_modelspace;
layout (location = 2) in vec2 vertexUV;

// common uniform
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;


// common output
out V_OUT
{
   vec3 Position_worldspace;
   vec3 EyeDirection_cameraspace;
   vec3 LightDirection_cameraspace;
   vec3 Normal_cameraspace;
   vec3 normal;
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

    // Normal of the the vertex, in camera space
    v_out.Normal_cameraspace  = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;
    v_out.UV = vertexPosition_modelspace.xy * 0.5 + 0.5;
};



void main() {
    commonOutput(
        vertexPosition_modelspace, 
        vertexNormal_modelspace
    );
}
