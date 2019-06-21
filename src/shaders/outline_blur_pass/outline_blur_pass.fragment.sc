$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texture, 0);

uniform vec4 u_outline_color;

void main() {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal coordinate system starts at upper-left corner.
    vec2 uv = v_texcoord0;
#else
    // OpenGL coordinate system starts at lower-left corner.
    vec2 uv = vec2(v_texcoord0.x, 1.0 - v_texcoord0.y);
#endif

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
