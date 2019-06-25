$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

void main() {
    v_texcoord0 = vec2(a_texcoord0.x, 1.0 - a_texcoord0.y);
    gl_Position = vec4(a_position.xy, 1.0, 1.0);
}
