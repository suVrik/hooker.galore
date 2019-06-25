$input v_position

#include <bgfx_shader.sh>

#define PI 3.14159265359

SAMPLERCUBE(s_texture, 0);
uniform float u_side_resolution;
uniform float u_roughness;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float VanDerCorpus(int n, int base) {
    float invBase = 1.0 / float(base);
    float denom   = 1.0;
    float result  = 0.0;

    for (int i = 0; i < 32; i++) {
        if (n > 0) {
            denom   = mod(float(n), 2.0);
            result += denom * invBase;
            invBase = invBase / 2.0;
            n       = int(float(n) / 2.0);
        }
    }

    return result;
}

vec2 Hammersley(int i, int N) {
    return vec2(float(i)/float(N), VanDerCorpus(i, 2));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
	float a = roughness*roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

void main() {
    vec3 normal = normalize(v_position);
    vec3 reflection = normal;
    vec3 camera = reflection;

    const int SAMPLE_COUNT = 1024;
    vec3 prefiltered_color = vec3(0.0, 0.0, 0.0);
    float total_weight = 0.0;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(xi, normal, u_roughness);
        vec3 L  = normalize(2.0 * dot(camera, H) * H - camera);

        float NdotL = max(dot(normal, L), 0.0);
        if (NdotL > 0.0) {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(normal, H, u_roughness);
            float NdotH = max(dot(normal, H), 0.0);
            float HdotV = max(dot(H, camera), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float sa_texel  = 4.0 * PI / (6.0 * u_side_resolution * u_side_resolution);
            float sa_sample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mip_level = u_roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);
            prefiltered_color += textureCubeLod(s_texture, L, mip_level).xyz * NdotL;
            total_weight += NdotL;
        }
    }
    gl_FragColor = vec4(prefiltered_color / total_weight, 1.0);
}
