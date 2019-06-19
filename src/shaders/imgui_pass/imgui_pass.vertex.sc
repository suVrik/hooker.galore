$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>

void main() {
    vec2 position = 2.0 * a_position.xy * u_viewTexel.xy;
    gl_Position = vec4(position.x - 1.0, 1.0 - position.y, 0.0, 1.0);
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
}
