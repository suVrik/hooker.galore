$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_position

#include <bgfx_shader.sh>
#include <shaderlib.sh>

SAMPLER2D(s_color_roughness, 0);
SAMPLER2D(s_normal_metal_ao, 1);
SAMPLER2D(s_parallax,        2);

uniform vec4 u_parallax_settings;

#define u_parallax_scale u_parallax_settings.x
#define u_parallax_steps u_parallax_settings.y
#define u_parallax_depth u_parallax_settings.z

vec2 parallax_uv(vec2 uv, vec3 view_dir) {
    float cur_layer_depth = 0.0;
    vec2 delta_uv = view_dir.xy * u_parallax_scale / (view_dir.z * u_parallax_steps);
    vec2 cur_uv = uv;

    float depth_from_tex;

    for (int i = 0; i < 32; i++) {
        cur_layer_depth += u_parallax_depth;
        cur_uv -= delta_uv;
        depth_from_tex = texture2D(s_parallax, cur_uv).r;
        if (depth_from_tex < cur_layer_depth) {
            break;
        }
    }

    vec2 prev_uv = cur_uv + delta_uv;
    float next = depth_from_tex - cur_layer_depth;
    float prev = texture2D(s_parallax, prev_uv).r - cur_layer_depth + u_parallax_depth;
    float weight = next / (next - prev);

    return mix(cur_uv, prev_uv, weight);
}

void main() {
    mat3 tangent_space_matrix = mat3(v_tangent, v_bitangent, v_normal);

    vec3 camera_position = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 tangent_view_direction = mul(tangent_space_matrix, camera_position.xyz - v_position);
    vec3 normalized_tangent_view_direction = normalize(tangent_view_direction);
    vec2 parallax_texcoord = parallax_uv(v_texcoord0, normalized_tangent_view_direction);

    vec4 normal_metal_ao = texture2D(s_normal_metal_ao, parallax_texcoord);

    vec3 normal;
    normal.x = normal_metal_ao.x * 2.0 - 1.0;
    normal.y = 1.0 - normal_metal_ao.y * 2.0;
    normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
    normal = normalize(mul(tangent_space_matrix, normal));

    gl_FragData[0] = texture2D(s_color_roughness, parallax_texcoord);
    gl_FragData[1] = vec4(encodeNormalOctahedron(normal), normal_metal_ao.zw);
}
