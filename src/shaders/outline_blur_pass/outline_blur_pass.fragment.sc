$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLER2D(s_texture, 0);

uniform vec4 u_outline_color;

void main() {
    vec2 uv = to_uv(v_texcoord0);
    gl_FragColor = (vec4(1.0, 1.0, 1.0, 1.0) - texture2D(s_texture, uv)) *
                   min(texture2D(s_texture, uv + vec2(u_viewTexel.x, 0.0)) +
                       texture2D(s_texture, uv - vec2(u_viewTexel.x, 0.0)) +
                       texture2D(s_texture, uv + vec2(0.0, u_viewTexel.y)) +
                       texture2D(s_texture, uv - vec2(0.0, u_viewTexel.y)) +
                       texture2D(s_texture, uv - vec2(u_viewTexel.x, u_viewTexel.y)) +
                       texture2D(s_texture, uv - vec2(u_viewTexel.x, -u_viewTexel.y)) +
                       texture2D(s_texture, uv - vec2(-u_viewTexel.x, u_viewTexel.y)) +
                       texture2D(s_texture, uv - vec2(-u_viewTexel.x, -u_viewTexel.y)) +
                       texture2D(s_texture, uv + vec2(u_viewTexel.x * 2.0, 0.0)) +
                       texture2D(s_texture, uv - vec2(u_viewTexel.x * 2.0, 0.0)) +
                       texture2D(s_texture, uv + vec2(0.0, u_viewTexel.y * 2.0)) +
                       texture2D(s_texture, uv - vec2(0.0, u_viewTexel.y * 2.0)),
                       vec4(1.0, 1.0, 1.0, 1.0)) * u_outline_color;
}
