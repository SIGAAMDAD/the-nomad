in vec3 a_Position;
in vec2 a_TexCoords;
in uvec2 a_WorldPos;
in vec4 a_Color;

out vec2 v_TexCoords;
out vec4 v_Color;
out uvec2 v_WorldPos;
out vec3 v_Position;

uniform mat4 u_ModelViewProjection;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;
uniform int u_ColorGen;
uniform int u_AlphaGen;

TEXTURE2D u_DiffuseMap;

void main() {
	v_TexCoords = a_TexCoords;

	if ( u_ColorGen == CGEN_VERTEX ) {
		v_Color = vec4( 1.0 );
	} else {
		v_Color = u_VertColor * a_Color + u_BaseColor;
	}
	v_WorldPos = a_WorldPos;
	v_Position = a_Position;
	
    gl_Position = u_ModelViewProjection * vec4( a_Position, 1.0 );
}