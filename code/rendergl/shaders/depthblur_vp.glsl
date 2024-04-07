in vec4 a_Position;
in vec4 a_LightCoords;

uniform vec4 u_ViewInfo; // zfar / znear, zfar, 1/width, 1/height

out vec2 v_ScreenTex;

void main() {
	gl_Position = a_Position;
	vec2 wh = vec2( 1.0 ) / u_ViewInfo.zw - vec2( 1.0 );
	v_ScreenTex = ( floor( a_LightCoords.xy * wh ) + vec2( 0.5 ) ) * u_ViewInfo.zw;

	//vec2 screenCoords = gl_Position.xy / gl_Position.w;
	//var_ScreenTex = screenCoords * 0.5 + 0.5;
}
