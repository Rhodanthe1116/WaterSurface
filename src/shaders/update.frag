#version 430 core
uniform sampler2D water;
uniform vec2 delta;

in vec2 coord;

void main() {
    // The data in the texture is (position.y, velocity.y, normal.x, normal.z)
    /* get vertex info */
    vec4 info = texture(water, coord);
      
    /* calculate average neighbor height */
    vec2 dx = vec2(delta.x, 0.0);
    vec2 dy = vec2(0.0, delta.y);
    float average = (
        texture(water, coord - dx).r +
        texture(water, coord - dy).r +
        texture(water, coord + dx).r +
        texture(water, coord + dy).r 
    ) / 4;
     
    info.g = info.g * 2 - 1;
    /* change the velocity to move toward the average */
    float d = (average - info.r);
    info.g += d * 2;
    

    /* attenuate the velocity a little so waves do not last forever */
    info.g *= 0.995;
      
    /* move the vertex along the velocity */
    info.r += info.g;
    info.r = average;
     
    info.g = (info.g + 1) / 2;
    gl_FragColor = info;
}