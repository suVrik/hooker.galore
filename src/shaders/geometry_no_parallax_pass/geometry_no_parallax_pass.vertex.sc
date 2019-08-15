$input a_position, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_position

#include <bgfx_shader.sh>

void main() {
    v_normal    = normalize(mul(u_model[0], vec4(a_normal.xyz,  0.0)).xyz);
    v_tangent   = normalize(mul(u_model[0], vec4(a_tangent.xyz, 0.0)).xyz);
    v_bitangent = cross(v_normal, v_tangent) * a_tangent.w;
    v_texcoord0 = a_texcoord0;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_position  = gl_Position;
}
