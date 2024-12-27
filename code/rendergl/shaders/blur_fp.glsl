#if defined(USE_MULTIATTRIB)
layout( location = 0 ) out vec4 a_Color;
#else
out vec4 a_Color;
#endif

in vec2 v_TexCoords;

TEXTURE2D u_DiffuseMap;
uniform bool u_BlurHorizontal;

void main() {
	vec2 texOffset = 1.0 / textureSize( u_DiffuseMap, 0 ); // gets size of single texel
	vec3 result = texture( u_DiffuseMap, v_TexCoords ).rgb * 0.2270270270;
	
	if ( result == vec3( 0.0, 0.0, 0.0 ) ) {
		discard;
	}
	if ( u_BlurHorizontal ) {
		result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * 1, 0.0 ) ).rgb * 0.1945945946;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * 1, 0.0 ) ).rgb * 0.1945945946;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * 2, 0.0 ) ).rgb * 0.1945945946;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * 2, 0.0 ) ).rgb * 0.1945945946;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * 3, 0.0 ) ).rgb * 0.1216216216;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * 3, 0.0 ) ).rgb * 0.1216216216;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * 4, 0.0 ) ).rgb * 0.0540540541;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * 4, 0.0 ) ).rgb * 0.0540540541;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( texOffset.x * 5, 0.0 ) ).rgb * 0.0162162162;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( texOffset.x * 5, 0.0 ) ).rgb * 0.0162162162;
	}
	else {
		result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.x * 1 ) ).rgb * 0.1945945946;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.x * 1 ) ).rgb * 0.1945945946;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.x * 2 ) ).rgb * 0.1945945946;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.x * 2 ) ).rgb * 0.1945945946;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.x * 3 ) ).rgb * 0.1216216216;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.x * 3 ) ).rgb * 0.1216216216;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.x * 4 ) ).rgb * 0.0540540541;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.x * 4 ) ).rgb * 0.0540540541;

		result += texture( u_DiffuseMap, v_TexCoords + vec2( 0.0, texOffset.x * 5 ) ).rgb * 0.0162162162;
		result += texture( u_DiffuseMap, v_TexCoords - vec2( 0.0, texOffset.x * 5 ) ).rgb * 0.0162162162;
	}
	a_Color = vec4( result, 1.0 );
}
