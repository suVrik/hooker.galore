vec3 v_normal        : NORMAL    = vec3(1.0, 0.0, 0.0);
vec3 v_tangent       : TANGENT   = vec3(0.0, 1.0, 0.0);
vec3 v_bitangent     : BINORMAL  = vec3(0.0, 0.0, 1.0);
vec2 v_texcoord0     : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_view_position : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3 v_frag_position : TEXCOORD2 = vec3(0.0, 0.0, 0.0);

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec4 a_tangent   : TANGENT;
vec2 a_texcoord0 : TEXCOORD0;
