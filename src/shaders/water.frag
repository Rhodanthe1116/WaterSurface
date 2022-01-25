#version 430 core

in V_OUT {
   vec3 Position_worldspace;
   vec3 EyeDirection_cameraspace;
   vec3 LightDirection_cameraspace;
   vec2 UV;
}
f_in;

out vec4 f_color;

// common uniform
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

uniform vec3 EyePosition_worldspace;

uniform vec3 LightPosition_worldspace;
uniform float LightPower;
uniform vec3 ambientLight;
uniform vec3 specularLight;
// Light emission properties
vec3 LightColor = vec3(1, 1, 1);

uniform samplerCube skybox_cube;
uniform samplerCube tiles_cube;
uniform sampler2D causticTex;
uniform sampler2D water;

uniform vec3 sphereCenter;
uniform float sphereRadius;
 
vec3 Normal_worldspace;
vec3 Normal_cameraspace;

const vec3 abovewaterColor = vec3(0.25, 1.0, 1.25);
const vec3 underwaterColor = vec3(0.1, 0.2, 0.5);
const float poolHeight = 1.0;
const float ratio_of_reflect_refract = 0.3;
const float IOR_AIR = 1.0;
const float IOR_WATER = 1.333;


vec3 cal_color() {  
    // Material properties
	// vec3 MaterialDiffuseColor = texture( texture_diffuse1, f_in.UV ).rgb;
	vec3 MaterialDiffuseColor = underwaterColor;
	vec3 MaterialAmbientColor = ambientLight * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = specularLight;

    float distanceToLight = length( LightPosition_worldspace - f_in.Position_worldspace );

    // Normal of the computed fragment, in camera space
    vec3 n = normalize( Normal_cameraspace );
    // Direction of the light (from the fragment to the light)
    vec3 l = normalize( f_in.LightDirection_cameraspace );
    // Cosine of the angle between the normal and the light direction,
    // clamped above 0
    //  - light is at the vertical of the triangle -> 1
    //  - light is perpendicular to the triangle -> 0
    //  - light is behind the triangle -> 0
    float cosTheta = clamp( dot( n,l ), 0,1 );

    // Eye vector (towards the camera)
	vec3 E = normalize(f_in.EyeDirection_cameraspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	vec3 color = 
		// Ambient : simulates indirect lighting
		MaterialAmbientColor +
		// Diffuse : "color" of the object
		MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distanceToLight) +
		// Specular : reflective highlight, like a mirror
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distanceToLight);
	
	return color;
}


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
float get_height(vec3 point) {
    vec4 info = texture(causticTex, point.xz * 0.5 + 0.5);
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
    vec4 info = texture(water, point.xz * 0.5 + 0.5);
    if (point.y < info.r) {
      // TODO: What's this?
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

vec3 getSurfaceRayColor(vec3 origin, vec3 ray, vec3 waterColor) {
    vec3 color;

    if (ray.y < 0.0) {
        vec2 t = intersectCube(origin, ray, vec3(-1.0, -poolHeight, -1.0), vec3(1.0, 2.0, 1.0));
        color = getWallColor(origin + ray * t.y);
    } else {
        vec2 t = intersectCube(origin, ray, vec3(-1.0, -poolHeight, -1.0), vec3(1.0, 2.0, 1.0));
        vec3 hit = origin + ray * t.y;
        if (hit.y < 2.0 / 12.0) {
            color = getWallColor(hit);
        } else {
            color = textureCube(skybox_cube, ray).rgb;
            color += vec3(pow(max(0.0, dot(LightPosition_worldspace, ray)), 5000.0)) * vec3(10.0, 8.0, 6.0);
        }
    }
    if (ray.y < 0.0) color *= waterColor;
    return color;
}


vec3 cal_reflect() {
	// Normal of the computed fragment, in camera space
    vec3 n = normalize( Normal_cameraspace ); 
    // Eye vector (towards the vertex)
	vec3 I = normalize(-f_in.EyeDirection_cameraspace);
    vec3 R = reflect(I, n);
    vec4 reflect_color = texture(skybox_cube, R);

	return vec3(reflect_color);
}

vec3 cal_refract() {
    vec3 n = normalize( Normal_worldspace ); 
	vec3 incomingRay = normalize(f_in.Position_worldspace - EyePosition_worldspace);
    vec3 refractedRay = refract(incomingRay, n, IOR_AIR / IOR_WATER);

	return getSurfaceRayColor(
            f_in.Position_worldspace, 
            refractedRay, 
            abovewaterColor
    );
}

void main() {
    vec2 coord = f_in.UV;
    vec4 info = texture(water, coord);
    Normal_worldspace = vec3(info.b, sqrt(1.0 - dot(info.ba, info.ba)), info.a);
    Normal_cameraspace  = (V * M * vec4(Normal_worldspace, 0)).xyz;


	vec3 reflect_color = cal_reflect();
	vec3 refractedColor = cal_refract();


	// vec3 color = cal_color();
    // f_color = vec4(color, 0.3);

	 
    // f_color = vec4(refractedColor, 1.0);

	vec3 incomingRay = normalize(f_in.Position_worldspace - EyePosition_worldspace);
    float fresnel = mix(0.25, 1.0, pow(1.0 - dot(Normal_worldspace, -incomingRay), 3.0));

    // f_color = vec4(refractedColor, 1.0);
    f_color = vec4(mix(refractedColor, reflect_color, fresnel), 1.0);
}
