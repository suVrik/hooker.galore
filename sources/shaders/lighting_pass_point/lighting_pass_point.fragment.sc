$input v_texcoord0

#include <bgfx_shader.sh>
#include <shaderlib.sh>
#include <shader_utils.sh>

SAMPLER2D(s_color_roughness,     0);
SAMPLER2D(s_normal_metal_ao,     1);
SAMPLER2D(s_depth,               2);

uniform vec4 u_light_color;
uniform vec4 u_light_position;
uniform vec4 u_mip_prefilter_max;

void main() {
    vec2 uv = to_uv(v_texcoord0);
    vec4 color_roughness = texture2D(s_color_roughness, uv);
    vec3 color = toLinear(color_roughness.xyz);
    float roughness = color_roughness.w;

    vec4 normal_metal_ao = texture2D(s_normal_metal_ao, uv);
    vec3 normal = decodeNormalOctahedron(normal_metal_ao.xy);
    float metal = normal_metal_ao.z;
    float ao = normal_metal_ao.w;

    float clip_depth = texture2D(s_depth, uv).x;
    vec3 clip_position = to_clip_space_position(vec3(uv * 2.0 - 1.0, clip_depth));
    vec3 world_position = to_world_space_position(clip_position);
    vec3 camera_position = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 camera_dir = normalize(camera_position.xyz - world_position);
    vec3 reflect_dir = reflect(-camera_dir, normal);

    vec3 surface_reflect_zero = vec3(0.04, 0.04, 0.04);
    surface_reflect_zero = mix(surface_reflect_zero, color, metal);

        vec3 light_dir = normalize(u_light_position.xyz - world_position);
        vec3 half_dir = normalize(camera_dir + light_dir);

        float distance = length(u_light_position.xyz - world_position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = u_light_color.xyz * attenuation;

        float ndf = distribution_ggx(normal, half_dir, roughness);
        float g = geometry_smith(normal, camera_dir, light_dir, roughness);
        vec3 f = fresnel_schlick(max(dot(half_dir, camera_dir), 0.0), surface_reflect_zero, 0.0);

        vec3 numerator = ndf * g * f;
        float denominator = 4.0 * max(dot(normal, camera_dir), 0.0) * max(dot(normal, light_dir), 0.0) + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kd = (vec3(1.0, 1.0, 1.0) - f) * (1.0 - metal);
        float dot_norm_light = max(dot(normal, light_dir), 0.0);
        vec3 outgoing_radiance = (kd * color / PI + specular) * radiance * dot_norm_light;

    //vec3 color_out = outgoing_radiance;
    vec3 color_out = vec3_splat(0.0);
    gl_FragColor = vec4(color_out, 1.0);
}
