#ifndef SHADER_UTILS_H_HEADER_GUARD
#define SHADER_UTILS_H_HEADER_GUARD

#define PI 3.14159265359

float van_der_corpus(int n, int base) {
    float inv_base = 1.0 / float(base);
    float denom   = 1.0;
    float result  = 0.0;

    for(int i = 0; i < 32; i++) {
        if (n > 0) {
            denom   = mod(float(n), 2.0);
            result += denom * inv_base;
            inv_base = inv_base / 2.0;
            n       = int(float(n) / 2.0);
        }
    }

    return result;
}

vec2 hammersley(int i, int N) {
    return vec2(float(i) / float(N), van_der_corpus(i, 2));
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

vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness) {
	float a = roughness * roughness;
	float phi = 2.0 * PI * xi.x;
	float cos_theta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	vec3 h;
	h.x = cos(phi) * sin_theta;
	h.y = sin(phi) * sin_theta;
	h.z = cos_theta;

	vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);

	vec3 sample_vec = tangent * h.x + bitangent * h.y + n * h.z;
	return normalize(sample_vec);
}

float to_texture_depth(float depth) {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal range for Z is [0, 1].
    return depth;
#else
    // OpenGL range for Z is [-1, 1].
    return depth * 0.5 + 0.5;
#endif
}

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

vec2 to_uv(float u, float v) {
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
    // DirectX & Metal coordinate system starts at upper-left corner.
    return vec2(u, v);
#else
    // OpenGL coordinate system starts at lower-left corner.
    return vec2(u, 1.0 - v);
#endif
}

vec2 to_uv(vec2 uv) {
    return to_uv(uv.x, uv.y);
}

#endif // SHADER_UTILS_H_HEADER_GUARD
