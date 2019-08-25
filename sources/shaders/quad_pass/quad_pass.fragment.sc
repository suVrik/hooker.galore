$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLER2D(s_texture, 0);

void main() {
    gl_FragColor = texture2D(s_texture, to_uv(v_texcoord0));
}
