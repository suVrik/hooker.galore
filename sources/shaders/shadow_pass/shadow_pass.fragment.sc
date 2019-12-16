$input v_position

#include <bgfx_shader.sh>
#include <shaderlib.sh>
#include <shader_utils.sh>

void main() {
    float depth_out = (v_position.z / v_position.w + 1.0) / 2.0;

    gl_FragColor = vec4(depth_out, depth_out, depth_out, 1.0);
}
