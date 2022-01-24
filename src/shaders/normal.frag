#version 430 core
uniform sampler2D water;
uniform vec2 delta;
in vec2 coord;
void main() {
    /* get vertex info */
    vec4 info = texture(water, coord);
      
    /* update the normal */
    vec3 dx = vec3(delta.x, texture(water, vec2(coord.x + delta.x, coord.y)).r - info.r, 0.0);
    vec3 dy = vec3(0.0, texture(water, vec2(coord.x, coord.y + delta.y)).r - info.r, delta.y);
    info.ba = normalize(cross(dy, dx)).xz;
      
    gl_FragColor = info;
}