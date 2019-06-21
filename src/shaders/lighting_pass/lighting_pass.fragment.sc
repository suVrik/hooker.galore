$input v_texcoord0

#include <bgfx_shader.sh>
#include <shaderlib.sh>

SAMPLER2D(s_color_roughness, 0);
SAMPLER2D(s_normal_metal_ao, 1);
SAMPLER2D(s_depth,           2);

uniform vec4 u_light_position;

float to_clip_space_depth(float depth) {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal range for Z is [0, 1].
    return depth;
#else
    // OpenGL range for Z is [-1, 1].
    return depth * 2.0 - 1.0;
#endif
}

vec3 to_clip_space_position(vec3 position) {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal coordinate system starts at upper-left corner.
    return vec3(position.x, -position.y, position.z);
#else
    // OpenGL coordinate system starts at lower-left corner.
    return position;
#endif
}

vec3 to_world_space_position(vec3 position) {
    vec4 result = mul(u_invViewProj, vec4(position, 1.0));
    return result.xyz / result.w;
}

void main() {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal coordinate system starts at upper-left corner.
    vec2 uv = v_texcoord0;
#else
    // OpenGL coordinate system starts at lower-left corner.
    vec2 uv = vec2(v_texcoord0.x, 1.0 - v_texcoord0.y);
#endif

    vec4 color_roughness = texture2D(s_color_roughness, uv);
    vec3 color = color_roughness.xyz;
    float roughness = color_roughness.w;

    vec4 normal_metal_ao = texture2D(s_normal_metal_ao, uv);
    vec3 normal = decodeNormalOctahedron(normal_metal_ao.xy);
    float metal = normal_metal_ao.z;
    float ao = normal_metal_ao.w;

    float clip_depth = to_clip_space_depth(texture2D(s_depth, uv).x);
    vec3 clip_position = to_clip_space_position(vec3(uv * 2.0 - 1.0, clip_depth));
    vec3 world_position = to_world_space_position(clip_position);

    vec3 light_dir = normalize(u_light_position.xyz - world_position);

    float diffuse = max(dot(light_dir, normal), 0.0);
    gl_FragColor = vec4(diffuse * color + color * 0.3, 1.0);
}
