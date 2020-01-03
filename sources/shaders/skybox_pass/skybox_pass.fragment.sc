$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLERCUBE(s_skybox, 0);

uniform mat4 u_rotation;

void main() {
    vec2 uv = to_uv(v_texcoord0);
    vec3 clip_position = to_clip_space_position(vec3(uv * 2.0 - 1.0, 1.0));
    mat4 mtx = mul(u_rotation, u_invProj);
    clip_position = normalize(mul(mtx, vec4(clip_position, 1.0)).xyz);
    vec3 color_out = textureCube(s_skybox, clip_position).xyz;
    gl_FragColor = vec4(color_out, 1.0);
}
