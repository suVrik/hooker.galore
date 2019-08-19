$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_position

#include <bgfx_shader.sh>
#include <shaderlib.sh>
#include <shader_utils.sh>

SAMPLER2D(s_color_roughness, 0);
SAMPLER2D(s_normal_metal_ao, 1);

void main() {
    #if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal treats vec3 as row vectors.
    mat3 from_tangent_space_matrix = transpose(mat3(v_tangent, v_bitangent, v_normal));
    #else
    // OpenGL treats vec3 as column vectors.
    mat3 from_tangent_space_matrix = mat3(v_tangent, v_bitangent, v_normal);
    #endif

    vec4 normal_metal_ao = texture2D(s_normal_metal_ao, v_texcoord0);

    vec3 normal;
    normal.xy = normal_metal_ao.xy * 2.0 - 1.0;
    normal.z = sqrt(1.0 - clamp(dot(normal.xy, normal.xy), 0.0, 1.0));
    normal = normalize(mul(from_tangent_space_matrix, normal));

    float depth_out = v_position.z / v_position.w;

    gl_FragData[0] = texture2D(s_color_roughness, v_texcoord0);
    gl_FragData[1] = vec4(encodeNormalOctahedron(normal), normal_metal_ao.zw);
    gl_FragData[2] = vec4(depth_out, 0.0, 0.0, 1.0);
}
