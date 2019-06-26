$input v_texcoord0

#include <bgfx_shader.sh>
#include <shaderlib.sh>

#define PI 3.14159265359

SAMPLER2D(s_color_roughness,     0);
SAMPLER2D(s_normal_metal_ao,     1);
SAMPLER2D(s_depth,               2);
SAMPLERCUBE(s_skybox,            3);
SAMPLERCUBE(s_skybox_irradiance, 4);
SAMPLERCUBE(s_skybox_prefilter,  5);
SAMPLER2D(s_skybox_lut,          6);

uniform vec4 u_light_position;
uniform vec4 u_light_color;
uniform mat4 u_rotation;
uniform vec4 u_mip_prefilter_max;

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

vec3 fresnel_schlick(float cos_theta, vec3 srz, float roughness) {
    return srz + (max(vec3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), srz) - srz) * pow(1.0 - cos_theta, 5.0);
}

float distribution_ggx(vec3 normal, vec3 h, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float dot_normal_half  = max(dot(normal, h), 0.0);
    float dot_normal_half2 = dot_normal_half * dot_normal_half;

    float denom = (dot_normal_half2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float geometry_schlick_ggx(float dot_normal_half, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = dot_normal_half * (1.0 - k) + k;
    return dot_normal_half / denom;
}
float geometry_smith(vec3 normal, vec3 camera_dir, vec3 light_dir, float roughness) {
    float dot_normal_camera = max(dot(normal, camera_dir), 0.0);
    float dot_normal_light = max(dot(normal, light_dir), 0.0);
    float ggx2 = geometry_schlick_ggx(dot_normal_camera, roughness);
    float ggx1 = geometry_schlick_ggx(dot_normal_light, roughness);
    return ggx1 * ggx2;
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
    vec3 color = pow(color_roughness.xyz, vec3(2.2, 2.2, 2.2));
    float roughness = color_roughness.w;

    vec4 normal_metal_ao = texture2D(s_normal_metal_ao, uv);
    vec3 normal = decodeNormalOctahedron(normal_metal_ao.xy);
    float metal = normal_metal_ao.z;
    float ao = normal_metal_ao.w;

    float clip_depth = to_clip_space_depth(texture2D(s_depth, uv).x);
    vec3 clip_position = to_clip_space_position(vec3(uv * 2.0 - 1.0, clip_depth));
    if (clip_depth != 1.0) {
        vec3 world_position = to_world_space_position(clip_position);
        vec3 camera_position = mul(u_invView, vec4(0.0, 0.0, 0.0, 1.0)).xyz;
        vec3 camera_dir = normalize(camera_position.xyz - world_position);
        vec3 reflect_dir = reflect(-camera_dir, normal);

        vec3 surface_reflect_zero = vec3(0.04, 0.04, 0.04);
        surface_reflect_zero = mix(surface_reflect_zero, color, metal);

        vec3 outgoing_radiance = vec3(0.0, 0.0, 0.0);
        {
            vec3 light_dir = normalize(u_light_position.xyz - world_position);
            vec3 half_dir = normalize(camera_dir + light_dir);

            float distance = length(u_light_position.xyz - world_position);
            float attenuation = 1.0 / (distance * distance);
            vec3 radiance = u_light_color.xyz * attenuation;

            float ndf = distribution_ggx(normal, half_dir, roughness);
            float g = geometry_smith(normal, camera_dir, light_dir, roughness);
            vec3 f = fresnel_schlick(max(dot(half_dir, camera_dir), 0.0), surface_reflect_zero, roughness);

            vec3 numerator = ndf * g * f;
            float denominator = 4.0 * max(dot(normal, camera_dir), 0.0) * max(dot(normal, light_dir), 0.0) + 0.001;
            vec3 specular = numerator / denominator;

            vec3 kd = (vec3(1.0, 1.0, 1.0) - f) * (1.0 - metal);
            float dot_norm_light = max(dot(normal, light_dir), 0.0);
            outgoing_radiance += (kd * color / PI + specular) * radiance * dot_norm_light;
        }

        vec3 ks = fresnel_schlick(max(dot(normal, camera_dir), 0.0), surface_reflect_zero, roughness);
        vec3 kd = (1.0 - ks) * (1.0 - metal);
        vec3 irradiance = textureCube(s_skybox_irradiance, normal).xyz;
        vec3 diffuse = irradiance * color;
        vec3 prefiltered_color = textureCubeLod(s_skybox_prefilter, reflect_dir, roughness * u_mip_prefilter_max.x).xyz;
        #if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
        float roughness_v = roughness;
        #else
        float roughness_v = 1.0 - roughness;
        #endif
        vec2 brdf = texture2D(s_skybox_lut, vec2(max(dot(normal, camera_dir), 0.0), roughness_v)).xy;
        vec3 specular = prefiltered_color * (ks * brdf.x + brdf.y);
        vec3 ambient = (kd * diffuse + specular) * ao;

        vec3 color_out = ambient + outgoing_radiance;
        color_out = color_out / (color_out + vec3(1.0, 1.0, 1.0));
        color_out = pow(color_out, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
        gl_FragColor = vec4(color_out, 1.0);
    } else {
        mat4 mtx = mul(u_rotation, u_invProj);
        clip_position = normalize(mul(mtx, vec4(clip_position, 1.0)).xyz);
        vec3 color_out = textureCube(s_skybox, clip_position).xyz;
        color_out = color_out / (color_out + vec3(1.0, 1.0, 1.0));
        color_out = pow(color_out, vec3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
        gl_FragColor = vec4(color_out, 1.0);
    }
}
