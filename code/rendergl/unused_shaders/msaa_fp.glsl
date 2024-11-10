layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;

#if defined(USE_BINDLESS_TEXTURE)
layout( bindless_sampler ) uniform sampler2DMS u_DiffuseMap;
#else
uniform sampler2DMS u_DiffuseMap;
#endif
uniform bool u_Bloom;
uniform vec2 u_ScreenSize;
uniform int u_AntiAliasingQuality;

in vec2 v_TexCoords;

void main() {
	const ivec2 coords = ivec2( u_ScreenSize.x * v_TexCoords.x, u_ScreenSize.y * v_TexCoords.y );

	switch ( u_AntiAliasingQuality ) {
	case QUALITY_LOW:
		for ( int i = 0; i < 2; i++ ) {
			const vec4 _sample = texelFetch( u_DiffuseMap, coords, i );
			a_Color += _sample;
		}
		a_Color /= 2;
		break;
	case QUALITY_NORMAL:
		for ( int i = 0; i < 4; i++ ) {
			const vec4 _sample = texelFetch( u_DiffuseMap, coords, i );
			a_Color += _sample;
		}
		a_Color /= 4;
		break;
	case QUALITY_HIGH:
		for ( int i = 0; i < 8; i++ ) {
			const vec4 _sample = texelFetch( u_DiffuseMap, coords, i );
			a_Color += _sample;
		}
		a_Color /= 8;
		break;
	};

	if ( u_Bloom ) {
		const float brightness = dot( a_Color.rgb, vec3( 0.1, 0.1, 0.1 ) );
		if ( brightness > 0.5 ) {
			a_BrightColor = vec4( a_Color.rgb, 1.0 );
		} else {
			a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
		}
	}
}