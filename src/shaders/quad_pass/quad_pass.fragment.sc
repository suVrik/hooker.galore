$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texture, 0);

void main() {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal coordinate system starts at upper-left corner.
    vec2 uv = v_texcoord0;
#else
    // OpenGL coordinate system starts at lower-left corner.
    vec2 uv = vec2(v_texcoord0.x, 1.0 - v_texcoord0.y);
#endif

    gl_FragColor = texture2D(s_texture, uv);
}
