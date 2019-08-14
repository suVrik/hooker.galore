$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLER2D(s_texture, 0);

uniform vec4 u_outline_color;

void main() {
    vec2 uv = to_uv(v_texcoord0);

	vec3 current_color = texture2D(s_texture, uv).xyz;
	vec3 left_color    = texture2D(s_texture, uv - vec2(u_viewTexel.x, 0.0)).xyz;
	vec3 right_color   = texture2D(s_texture, uv + vec2(u_viewTexel.x, 0.0)).xyz;
	vec3 top_color     = texture2D(s_texture, uv - vec2(0.0, u_viewTexel.y)).xyz;
	vec3 bottom_color  = texture2D(s_texture, uv + vec2(0.0, u_viewTexel.y)).xyz;

	if (length(current_color - left_color) + length(current_color - right_color) + length(current_color - top_color) + length(current_color - bottom_color) > 0.0) {
		gl_FragColor = u_outline_color;
	} else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
}
