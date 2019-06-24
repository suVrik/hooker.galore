$input v_position

#include <bgfx_shader.sh>

#define PI 3.14159265359

SAMPLERCUBE(s_texture, 0);

void main() {
    vec3 normal = normalize(v_position);
    vec3 irradiance = vec3(0.0, 0.0, 0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);

    float nr_samples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += 0.025) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += 0.01) {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sample_cec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

            irradiance += textureCube(s_texture, sample_cec).xyz * cos(theta) * sin(theta);
            nr_samples++;
        }
    }
    gl_FragColor = vec4(PI * irradiance / nr_samples, 1.0);
}
