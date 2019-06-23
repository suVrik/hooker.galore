$input a_position
$output v_position

#include <bgfx_shader.sh>

void main() {
    v_position = a_position;
    gl_Position = mul(u_viewProj, vec4(a_position, 1.0));
}
