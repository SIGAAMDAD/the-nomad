in vec3 a_Position;
in vec2 a_TexCoords;

out vec2 v_ScreenTex;

void main() {
	gl_Position = vec4( a_Position, 1.0 );
	v_ScreenTex = a_TexCoords;
	//vec2 screenCoords = gl_Position.xy / gl_Position.w;
	//var_ScreenTex = screenCoords * 0.5 + 0.5;
}
