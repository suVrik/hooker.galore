$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_bitangent, v_texcoord0

#include <bgfx_shader.sh>

void main() {
    v_normal    = normalize(mul(u_model[0], vec4(a_normal.xyz,  0.0)).xyz);
    v_tangent   = normalize(mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz);
    v_bitangent = cross(v_normal, v_tangent) * a_tangent.w;

    #if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal treats vec3 as row vectors.
    mat4 model_matrix = transpose(u_model[0]);
    #else
    // OpenGL treats vec3 as column vectors.
    mat4 model_matrix = u_model[0];
    #endif

    float scale_x = length(model_matrix[0].xyz);
    float scale_y = length(model_matrix[1].xyz);
    float scale_z = length(model_matrix[2].xyz);

    if (a_normal.x == 1.0) {
        v_texcoord0 = vec2(a_texcoord0.x * scale_z, a_texcoord0.y * scale_y);
    } else if (a_normal.x == -1.0) {
        v_texcoord0 = vec2((a_texcoord0.x + 1.0) * scale_z, a_texcoord0.y * scale_y);
    } else if (a_normal.y == 1.0) {
        v_texcoord0 = vec2(a_texcoord0.x * scale_x, (a_texcoord0.y - 1.0) * scale_z);
    } else if (a_normal.y == -1.0) {
        v_texcoord0 = vec2((a_texcoord0.x + 1.0) * scale_x, (a_texcoord0.y - 1.0) * scale_z);
    } else if (a_normal.z == 1.0) {
        v_texcoord0 = vec2((a_texcoord0.x + 1.0) * scale_x, a_texcoord0.y * scale_y);
    } else {
        v_texcoord0 = vec2(a_texcoord0.x * scale_x, a_texcoord0.y * scale_y);
    }

    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
