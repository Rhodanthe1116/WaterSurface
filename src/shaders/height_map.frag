#version 430 core
#extension GL_NV_shadow_samplers_cube : enable
// not use

in V_OUT {
   vec3 Position_worldspace;
   vec3 EyeDirection_cameraspace;
   vec3 LightDirection_cameraspace;
   vec3 Normal_cameraspace;
   vec3 normal;
   vec2 UV;
}
f_in;

out vec4 f_color;

uniform vec3 LightPosition_worldspace;
uniform float LightPower;
uniform vec3 ambientLight;
uniform vec3 specularLight;
// Light emission properties
vec3 LightColor = vec3(1, 1, 1);

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;


const vec3 underwaterColor = vec3(0.4, 0.9, 1.0);


vec3 cal_color() {  
    // Material properties
	// vec3 MaterialDiffuseColor = texture( texture_diffuse1, f_in.UV ).rgb;
	vec3 MaterialDiffuseColor = underwaterColor;
	vec3 MaterialAmbientColor = ambientLight * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = specularLight * vec3(0.1,0.1,0.1);

    float distanceToLight = length( LightPosition_worldspace - f_in.Position_worldspace );

    // Normal of the computed fragment, in camera space
    vec3 n = normalize( f_in.Normal_cameraspace );
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


void main() {
  
	vec3 color = cal_color();
    f_color = vec4(color, 0.3);
}
