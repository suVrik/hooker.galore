#include <bgfx_shader.sh>

uniform vec4 u_object_index;

void main() {
    gl_FragColor = u_object_index;
}
