#if defined(USE_MULTIATTRIB)
layout( location = 0 ) out vec4 a_Color;
#else
out vec4 a_Color;
#endif

in vec2 v_TexCoords;

TEXTURE2D u_DiffuseMap;
uniform bool u_BlurHorizontal;

void main() {
	float weights[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );
	vec2 texOffset = 1.0 / textureSize( u_DiffuseMap, 0 ); // gets size of single texel
	vec3 result = texture( u_DiffuseMap, v_TexCoords ).rgb * weights[0];
	
	if ( result == vec3( 0.0, 0.0, 0.0 ) ) {
		discard;
	}
	if ( u_BlurHorizontal ) {
		for ( int i = 1; i < 5; ++i ) {
		   result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * i, 0.0 ) ).rgb * weights[i];
		   result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * i, 0.0 ) ).rgb * weights[i];
		}
	}
	else {
		for ( int i = 1; i < 5; ++i ) {
			result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.y * i ) ).rgb * weights[i];
			result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.y * i ) ).rgb * weights[i];
		}
	}
	a_Color = vec4( result, 1.0 );
}
