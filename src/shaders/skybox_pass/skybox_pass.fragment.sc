$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLER2D(s_depth,    0);
SAMPLERCUBE(s_skybox, 1);

uniform mat4 u_rotation;

void main() {
    vec2 uv = to_uv(v_texcoord0);
    float clip_depth = texture2D(s_depth, uv).x;
    vec3 clip_position = to_clip_space_position(vec3(uv * 2.0 - 1.0, clip_depth));
    mat4 mtx = mul(u_rotation, u_invProj);
    clip_position = normalize(mul(mtx, vec4(clip_position, 1.0)).xyz);
    vec3 color_out = textureCube(s_skybox, clip_position).xyz;
    gl_FragColor = vec4(color_out, 1.0);
}
