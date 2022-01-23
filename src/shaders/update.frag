uniform sampler2D water;
uniform vec2 delta;
varying vec2 coord;
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
    ) * 0.25;
      
    /* change the velocity to move toward the average */
    info.g += (average - info.r) * 2.0;
      
    /* attenuate the velocity a little so waves do not last forever */
    info.g *= 0.995;
      
    /* move the vertex along the velocity */
    info.r += info.g;
      
    gl_FragColor = info;
}