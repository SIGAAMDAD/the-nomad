#if !defined(GLSL_LEGACY)
out vec4 a_Color;
#endif

uniform sampler2D u_DiffuseMap;
uniform vec4 u_Color;

in vec2 v_TexCoords;


void main() {
	a_Color = texture2D( u_DiffuseMap, v_TexCoords ) * u_Color;
}
