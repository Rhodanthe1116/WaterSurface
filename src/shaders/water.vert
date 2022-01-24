#version 430 core
// common input
layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal_modelspace;
layout (location = 2) in vec2 vertexUV;

// common uniform
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 LightPosition_worldspace;

uniform sampler2D water;
out vec3 position;
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

    // gl_Position = P * V * M * vec4(vertexPosition_modelspace, 1);
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

void main() {
	vec4 info = texture(water, vertexUV);
	position = vertexPosition_modelspace.xyz;
	position.y += info.r;
	gl_Position = P * V * M * vec4(position, 1.0);

    commonOutput(
        position, 
        position
    );
}