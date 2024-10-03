in vec3 a_Position;
in vec2 a_TexCoords;

out vec4 v_Position;
out vec4 v_TexCoord0;
out vec4 v_TexCoord1;
out vec4 v_TexCoord2;
out vec4 v_TexCoord3;
out vec4 v_TexCoord4;

uniform mat4 u_ModelViewProjection;
uniform vec2 u_ScreenSize;

void main() {
	v_TexCoord0.xy = a_TexCoords.xy;

	const vec4 SourceSize = vec4( u_ScreenSize, 1.0 / u_ScreenSize );

	vec2 ps = vec2(1.0/((u_ScreenSize.x == 0.0) ? SourceSize.x : u_ScreenSize.x), 1.0/((u_ScreenSize.y == 0.0) ? SourceSize.y : u_ScreenSize.y));
	float dx = ps.x * 0.5;
	float dy = ps.y * 0.5;
	
	v_TexCoord1.xy = v_TexCoord0.xy + vec2( -dx,  0 );
	v_TexCoord2.xy = v_TexCoord0.xy + vec2(  dx,  0 );
	v_TexCoord3.xy = v_TexCoord0.xy + vec2(  0, -dy );
	v_TexCoord4.xy = v_TexCoord0.xy + vec2(  0,  dy );
	v_TexCoord1.zw = v_TexCoord0.xy + vec2( -dx,-dy );
	v_TexCoord2.zw = v_TexCoord0.xy + vec2( -dx, dy );
	v_TexCoord3.zw = v_TexCoord0.xy + vec2(  dx,-dy );
	v_TexCoord4.zw = v_TexCoord0.xy + vec2(  dx, dy );

	gl_Position = u_ModelViewProjection * vec4( v_Position, 1.0 );
}