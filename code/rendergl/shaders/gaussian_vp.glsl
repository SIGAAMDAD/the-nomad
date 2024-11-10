in vec3 a_Position;
in vec2 a_TexCoords;

out vec2 v_Position;

void main() {
	v_Position.xy = a_Position.xy;
	gl_Position = vec4( a_Position, 1.0 );
}