in vec3 a_Position;
in vec2 a_TexCoords;
in uvec2 a_WorldPos;
in vec4 a_Color;

out vec2 v_TexCoords;
out vec4 v_Color;
out vec3 v_WorldPos;
out vec3 v_Position;

uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelMatrix;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;
uniform int u_ColorGen;
uniform int u_AlphaGen;

#if defined(USE_TCGEN)
uniform vec4 u_DiffuseTexMatrix;
uniform vec4 u_DiffuseTexOffTurb;
#endif

#if defined(USE_TCGEN)
uniform int u_TCGen0;
uniform vec3 u_TCGen0Vector0;
uniform vec3 u_TCGen0Vector1;
uniform vec3 u_WorldPos;
#endif

TEXTURE2D u_DiffuseMap;

void main() {
	v_TexCoords = a_TexCoords;

	if ( u_ColorGen == CGEN_VERTEX ) {
		v_Color = vec4( 1.0 );
	} else {
		v_Color = u_VertColor * a_Color + u_BaseColor;
	}
	v_WorldPos = vec3( a_WorldPos.xy, 0.0 );
	v_Position = a_Position;
	
    gl_Position = u_ModelViewProjection * vec4( a_Position, 1.0 );
}