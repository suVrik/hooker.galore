$input v_position

#include <bgfx_shader.sh>

SAMPLER2D(s_texture, 0);

vec2 sample_spherical_map(vec3 v) {
    vec2 uv1 = vec2(atan2(v.z, v.x), asin(v.y));
    uv1 *= vec2(0.1591, 0.3183);
    uv1 += 0.5;
    return uv1;
}

void main() {
    vec2 uv = sample_spherical_map(normalize(v_position));
    uv = vec2(1.0 - uv.x, uv.y);
    gl_FragColor = vec4(texture2D(s_texture, uv).xyz, 1.0);
}
