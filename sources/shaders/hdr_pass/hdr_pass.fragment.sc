$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>
#include <shaderlib.sh>

SAMPLER2D(s_texture, 0);

void main() {
    vec3 color = texture2D(s_texture, to_uv(v_texcoord0)).xyz;
    //color = toReinhard(color);
    gl_FragColor = vec4(color, 1.0);
}
