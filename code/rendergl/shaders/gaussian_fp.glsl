layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;

in vec2 v_Position;

TEXTURE2D u_DiffuseMap;
TEXTURE2D u_BrightMap;
uniform vec2 u_ScreenSize;
uniform bool u_BlurHorizontal;

void main() {
	const float radius = 15.0;
	float x, y, rr = radius * radius;
	float d, w, w0;

	a_BrightColor = texture( u_BrightMap, v_Position );

	vec2 p = 0.5 * ( vec2( 1.0, 1.0 ), + v_Position );
	a_Color = vec4( 0.0, 0.0, 0.0, 0.0 );
	w0 = 0.5135 / pow( radius, 0.96 );
	if ( u_BlurHorizontal ) {
		for ( d = 1.0 / u_ScreenSize.x, x = -radius, p.x += x * d; x <= radius; x++, p.x += d ) {
			w = w0 * exp( ( -x * x ) / ( 2.0 * rr ) );
			a_Color += texture( u_DiffuseMap, p ) * w;
		}
	}
	else {
		for ( d = 1.0 / u_ScreenSize.y, y = -radius, p.y += y * d; y <= radius; y++, p.y += d ) {
			w = w0 * exp( ( -y * y ) / ( 2.0 * rr ) );
			a_Color += texture( u_DiffuseMap, p ) * w;
		}
	}
}