$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texture, 0);

void main() {
    vec4 alpha = texture2D(s_texture, v_texcoord0);
    gl_FragColor = vec4(v_color0, alpha.r);
}
