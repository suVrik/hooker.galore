#include <bgfx_shader.sh>

uniform vec4 u_group_index;

void main() {
    gl_FragColor = u_group_index;
}
