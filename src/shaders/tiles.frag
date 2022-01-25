#version 330 core
in V_OUT {
   vec3 Position_worldspace;
}
f_in;

in vec3 TexCoords;
out vec4 color;

uniform samplerCube tiles_cube;
uniform sampler2D causticTex;
uniform sampler2D water;
uniform vec3 LightPosition_worldspace;
uniform int is_height_map;


const vec3 underwaterColor = vec3(0.4, 0.9, 1.0);
const float poolHeight = 1.0;
const float IOR_AIR = 1.0;
const float IOR_WATER = 1.333;


vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax) {
    vec3 tMin = (cubeMin - origin) / ray;
    vec3 tMax = (cubeMax - origin) / ray;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}
float AMPLITUDE = 0.15;
float HEIGHT_MAP_Y_OFFSET = 0.5;
float AMPLITUDE_BASE = 3;
float get_height() {
    vec4 info = texture(causticTex, f_in.Position_worldspace.xz * 0.5 + 0.5);
    return (info.r- HEIGHT_MAP_Y_OFFSET) * AMPLITUDE * AMPLITUDE_BASE;
}

vec3 getWallColor(vec3 point) {
    float scale = 0.5;
    
    vec3 wallColor;
    vec3 normal;
    if (abs(point.x) > 0.999) {
      wallColor = texture(tiles_cube, vec3(point.yz, 1)).rgb;
      normal = vec3(-point.x, 0.0, 0.0);
    } else if (abs(point.z) > 0.999) {
      wallColor = texture(tiles_cube, vec3(point.yx, 1)).rgb;
      normal = vec3(0.0, 0.0, -point.z);
    } else {
      wallColor = texture(tiles_cube, vec3(point.xz, 1)).rgb;
      normal = vec3(0.0, 1.0, 0.0);
    }
    
    scale /= length(point); /* pool ambient occlusion */
    // scale *= 1.0 - 0.9 / pow(length(point - sphereCenter) / sphereRadius, 4.0); /* sphere ambient occlusion */
    
    /* caustics */
    
    vec3 refractedLight = normalize(-refract(-LightPosition_worldspace, vec3(0.0, 1.0, 0.0), IOR_AIR / IOR_WATER));
    float diffuse = max(0.0, dot(refractedLight, normal));
    // vec4 info = texture(water, point.xz * 0.5 + 0.5);
    vec4 info = texture(causticTex, point.xz * 0.5 + 0.5);
    if (point.y < -99) {
      vec4 caustic = texture(causticTex, 0.75 * (point.xz - point.y * refractedLight.xz / refractedLight.y) * 0.5 + 0.5);
      scale += diffuse * caustic.r * 2.0 * caustic.g;
    } else {
      // shadow for the rim of the pool 
      // shadow?? light??
      vec2 t = intersectCube(point, refractedLight, vec3(-1.0, -poolHeight, -1.0), vec3(1.0, 2.0, 1.0));
      diffuse *= 1.0 / (1.0 + exp(-200.0 / (1.0 + 10.0 * (t.y - t.x)) * (point.y + refractedLight.y * t.y - 2.0 / 12.0)));
      
      scale += diffuse * 0.5;
    }
  
    return wallColor * scale;
  }


void main()
{
    color = vec4(getWallColor(f_in.Position_worldspace), 1.0);
    vec4 info = texture(water, f_in.Position_worldspace.xz * 0.5 + 0.5);
    //vec4 info = texture(causticTex, f_in.Position_worldspace.xz * 0.5 + 0.5);
    if (is_height_map == 1) {
        info.r = (info.r- HEIGHT_MAP_Y_OFFSET) * AMPLITUDE * AMPLITUDE_BASE;
    }
    if (f_in.Position_worldspace.y < info.r) {
    //if (f_in.Position_worldspace.y < get_height()) {
        color.rgb *= underwaterColor * 1.2;
    }
}