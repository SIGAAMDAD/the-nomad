#define mad( a, b, c ) ( a * b + c )

in vec3 a_Position;
in vec2 a_TexCoords;

uniform vec2 u_ScreenSize;

out vec2 v_TexCoords;
out vec4 v_Offset[3];

void main() {
	vec4 SMAA_RT_METRICS = vec4( 1.0 / u_ScreenSize.x, 1.0 / u_ScreenSize.y, u_ScreenSize.x, u_ScreenSize.y );

//    v_TexCoords = vec2( ( a_Position + 1.0 ) / 2.0 );
	v_TexCoords = a_TexCoords;

	v_Offset[0] = mad( SMAA_RT_METRICS.xyxy, vec4( -1.0, 0.0, 0.0, -1.0 ), v_TexCoords.xyxy );
	v_Offset[1] = mad( SMAA_RT_METRICS.xyxy, vec4(  1.0, 0.0, 0.0,  1.0 ), v_TexCoords.xyxy );
	v_Offset[2] = mad( SMAA_RT_METRICS.xyxy, vec4( -2.0, 0.0, 0.0, -2.0 ), v_TexCoords.xyxy );

	gl_Position = vec4( a_Position.xy, 0.0, 1.0 );
}
