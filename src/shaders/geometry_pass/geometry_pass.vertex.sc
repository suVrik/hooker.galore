$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_view_position, v_frag_position

#include <bgfx_shader.sh>

void main() {
    v_normal        = mul(u_model[0], vec4(a_normal.xyz,  0.0)).xyz;
    v_tangent       = mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz;
    v_bitangent     = cross(v_normal, v_tangent);

    v_view_position = mul(u_view, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    v_frag_position = mul(u_model[0], vec4(a_position, 1.0)).xyz;

    v_texcoord0     = a_texcoord0;

    gl_Position     = mul(u_modelViewProj, vec4(a_position, 1.0));
}
