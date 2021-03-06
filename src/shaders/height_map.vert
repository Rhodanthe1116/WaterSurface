#version 430 core
#define pi = 3.14159;

// common input
layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal_modelspace;
layout (location = 2) in vec2 vertexUV;

// common uniform
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 LightPosition_worldspace;

uniform sampler2D height_map;

// sinewave
uniform float amplitude, wavelength, time, speed, interactive_amplitude,
    interactive_wavelength, interactive_speed, interactive_radius;
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

float AMPLITUDE = 0.15;
float HEIGHT_MAP_Y_OFFSET = 0.5;
float AMPLITUDE_BASE = 3;
float get_height_of_uv_with_offset(ivec2 off) {
    return (textureOffset(height_map, vertexUV, off).r - HEIGHT_MAP_Y_OFFSET) * AMPLITUDE * AMPLITUDE_BASE;
    // return texture(texture_diffuse1, vertexUV).r - height_map_height_offset;
}

vec3 cal_vertexNormal_modelspace(sampler2D height_map_texture) {
    vec3 normal;
    const vec2 size = vec2(2.0,0.0);
    const ivec3 off = ivec3(-5,0,5);
    vec4 wave = texture(height_map_texture, vertexUV);
    float s11 = get_height_of_uv_with_offset(ivec2(0, 0));
    float s01 = get_height_of_uv_with_offset(off.xy);
    float s21 = get_height_of_uv_with_offset(off.zy);
    float s10 = get_height_of_uv_with_offset(off.yx); 
    float s12 = get_height_of_uv_with_offset(off.yz);
    vec3 va = normalize(vec3(size.x, s21-s01, size.y));      
    vec3 vb = normalize(vec3(size.y, s12-s10, -size.x));
    normal = cross(va, vb);
    return normal;
}


void main() {
    // vec3 height_map = vertexPosition_modelspace;
    // magic. should be cal by min(height_map_pixels)
    //float tmp_height =
    //    (texture(texture_diffuse1, texture_coordinate / wavelength).r - 0.5) *
    //    amplitude;
    float height_of_vertex = get_height_of_uv_with_offset(ivec2(0, 0));

    // float tmp_interactive = 0.0;
    // if (drop_point.x > 0.0) {
    //     float dist = distance(vertexUV, drop_point) /
    //                  interactive_wavelength * 100;
    //     float t_c = (time - drop_time) * (interactive_radius * 3.1415926) *
    //                 interactive_speed;
    //     tmp_interactive = interactive_amplitude *
    //                       sin((dist - t_c) * clamp(0.0125 * t_c, 0, 1)) /
    //                       (exp(0.1 * abs(dist - t_c) + (0.05 * t_c))) * 1.5;
    // }
    // if ((height_of_vertex < 0 && tmp_interactive > 0) ||
    //     (height_of_vertex > 0 && tmp_interactive < 0)) {
    //     height_map.y += (height_of_vertex + tmp_interactive);
    // } else {
    //     height_map.y += (abs(height_of_vertex) > abs(tmp_interactive))
    //                         ? height_of_vertex
    //                         : tmp_interactive;
    // }

    vec3 new_vertexPosition_modelspace = vec3(
        vertexPosition_modelspace.x, 
        height_of_vertex, 
        vertexPosition_modelspace.z
    );

    vec3 new_vertexNormal_modelspace = cal_vertexNormal_modelspace(height_map);
 
    commonOutput(
        new_vertexPosition_modelspace, 
        new_vertexNormal_modelspace
    );
}
