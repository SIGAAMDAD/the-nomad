in vec3 a_Position;
in vec2 a_TexCoords;

uniform mat4 u_ModelViewProjection;

out vec2 v_TexCoords;

void main() {
	gl_Position = u_ModelViewProjection * vec4( a_Position, 1.0 );
	v_TexCoords = a_TexCoords;
}
