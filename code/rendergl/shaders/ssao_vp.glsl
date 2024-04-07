in vec4 a_Position;
in vec4 a_LightCoords;

out vec2 v_ScreenTex;

void main() {
	gl_Position = a_Position;
	v_ScreenTex = a_TexCoords.xy;
	//vec2 screenCoords = gl_Position.xy / gl_Position.w;
	//var_ScreenTex = screenCoords * 0.5 + 0.5;
}
