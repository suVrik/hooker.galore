$input v_texcoord0

#include <bgfx_shader.sh>
#include <shaderlib.sh>
#include <shader_utils.sh>

SAMPLER2D(s_color_roughness,     0);
SAMPLER2D(s_normal_metal_ao,     1);
SAMPLER2D(s_depth,               2);
SAMPLERCUBE(s_skybox_irradiance, 3);
SAMPLERCUBE(s_skybox_prefilter,  4);
SAMPLER2D(s_skybox_lut,          5);
SAMPLER2D(s_shadow_map,          6);

uniform mat4 u_dir_light_view_proj;
uniform vec4 u_dir_light_position;
uniform vec4 u_light_color;
uniform vec4 u_light_position;
uniform vec4 u_mip_prefilter_max;

float get_shadow(vec4 shadow_space_frag_pos, vec3 normal, vec3 world_pos) {
    vec3 proj_coords = shadow_space_frag_pos.xyz / shadow_space_frag_pos.w;
    // transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;

    float current_depth = proj_coords.z;
    float shadow = 0.0;

    float bias = max(0.003 * (1.0 - dot(normal, -normalize(u_dir_light_position.xyz))), 0.001);

    //float bias = 0.005 * tan(acos(dot(normal, -normalize(u_dir_light_position.xyz)))); // cosTheta is dot( n,l ), clamped between 0 and 1
    //bias = clamp(bias, 0,0.01);

    vec2 texelSize = 1.0 / textureSize(s_shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture2D(s_shadow_map, proj_coords.xy + vec2(x, y) * texelSize).r;
            shadow += current_depth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return 1.0 - shadow;
}

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

    vec3 light_dir = normalize(u_light_position.xyz);
    vec3 half_dir = normalize(camera_dir + light_dir);

    //TODO pass a meaningful attenuation value in a uniform
    float attenuation = 0.075;
    vec3 radiance = u_light_color.xyz * attenuation;

    float ndf = distribution_ggx(normal, half_dir, roughness);
    float g = geometry_smith(normal, camera_dir, light_dir, roughness);
    vec3 f = fresnel_schlick(max(dot(half_dir, camera_dir), 0.0), surface_reflect_zero, 0.0);

    vec3 numerator = ndf * g * f;
    float denominator = 4.0 * max(dot(normal, camera_dir), 0.0) * max(dot(normal, light_dir), 0.0);
    vec3 specular_direct = numerator / max(denominator, 0.001);

    vec3 ks_dir = fresnel_schlick(max(dot(half_dir, camera_dir), 0.0), surface_reflect_zero, 0.0);
    vec3 kd_dir = (vec3_splat(1.0) - f) * (1.0 - metal);
    float dot_norm_light = max(dot(normal, light_dir), 0.0);
    vec3 directional = (kd_dir * (color / PI)) * radiance * dot_norm_light;

    vec3 ks = fresnel_schlick(max(dot(normal, camera_dir), 0.0), surface_reflect_zero, roughness);
    vec3 kd = (vec3_splat(1.0) - ks) * (1.0 - metal);
    vec3 irradiance = textureCube(s_skybox_irradiance, normal).xyz;
    vec3 diffuse = irradiance * color;
    vec3 prefiltered_color = textureCubeLod(s_skybox_prefilter, reflect_dir, roughness * u_mip_prefilter_max.x).xyz;
    vec2 brdf = texture2D(s_skybox_lut, vec2(max(dot(normal, camera_dir), 0.0), roughness)).xy;
    vec3 specular = prefiltered_color * (ks * brdf.x + brdf.y);
    vec3 ambient = (kd * diffuse + specular) * ao;

    vec4 shadow_space_frag_pos = mul(u_dir_light_view_proj, vec4(world_position, 1.0));
    float shadow = get_shadow(shadow_space_frag_pos, normal, world_position);

    vec3 color_out = vec3_splat(shadow);
    gl_FragColor = vec4(color_out, 1.0);
}
