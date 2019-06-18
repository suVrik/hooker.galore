$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_view_position, v_frag_position

#include <bgfx_shader.sh>
#include <shaderlib.sh>

SAMPLER2D(s_color_roughness, 0);
SAMPLER2D(s_normal_metal_ao, 1);
SAMPLER2D(s_parallax,        2);

vec2 parallax_uv(vec2 uv, vec3 view_dir) {
    const float u_parallax_scale = 10.0;
    const float u_num_steps = 32.0;

	float depth_scale = float(u_parallax_scale) / 1000.0;

    float layer_depth = 1.0 / float(u_num_steps);
    float cur_layer_depth = 0.0;
    vec2 delta_uv = view_dir.xy * depth_scale / (view_dir.z * u_num_steps);
    vec2 cur_uv = uv;

    float depth_from_tex = texture2D(s_parallax, cur_uv).r;

    for (int i = 0; i < 32; i++) {
        cur_layer_depth += layer_depth;
        cur_uv -= delta_uv;
        depth_from_tex = texture2D(s_parallax, cur_uv).r;
        if (depth_from_tex < cur_layer_depth) {
            break;
        }
    }

    vec2 prev_uv = cur_uv + delta_uv;
    float next = depth_from_tex - cur_layer_depth;
    float prev = texture2D(s_parallax, prev_uv).r - cur_layer_depth + layer_depth;
    float weight = next / (next - prev);

    return mix(cur_uv, prev_uv, weight);
}

void main() {
    mat3 tangent_space_matrix     = mat3(v_tangent, v_bitangent, v_normal);
    mat3 inv_tangent_space_matrix = transpose(tangent_space_matrix);

    vec3 view_direction = normalize(v_view_position - v_frag_position);
    vec3 tangent_view_direction = mul(inv_tangent_space_matrix, view_direction);
    vec2 parallax_texcoord     = parallax_uv(v_texcoord0, tangent_view_direction);

    vec4 normal_metal_ao      = texture2D(s_normal_metal_ao, parallax_texcoord);

    vec3 normal;
    normal.xy = normal_metal_ao.xy * 2.0 - 1.0;
    normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
    normal = normalize(mul(tangent_space_matrix, normal));

    gl_FragData[0] = vec4(tangent_view_direction * 0.5 + 0.5, 1.0); // texture2D(s_color_roughness, parallax_texcoord);
    gl_FragData[1] = vec4(encodeNormalOctahedron(normal), normal_metal_ao.zw);
}
