#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#endif

#if !defined(SMAA_THRESHOLD)
#define SMAA_THRESHOLD 0.1
#endif

#if !defined(SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR)
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0
#endif

uniform sampler2D u_DiffuseMap;

in vec2 v_TexCoords;
in vec4 v_Offset[3];

void main() {
	vec2 threshold = vec2( SMAA_THRESHOLD );

	// Calculate lumas:
	vec3 weights = vec3( 0.2126, 0.7152, 0.0722 );
	float L = dot( texture2D( u_DiffuseMap, v_TexCoords ).rgb, weights );

	float Lleft = dot( texture2D( u_DiffuseMap, v_Offset[0].xy ).rgb, weights );
	float Ltop  = dot( texture2D( u_DiffuseMap, v_Offset[0].zw ).rgb, weights );

	// We do the usual threshold:
	vec4 delta;
	delta.xy = abs( L - vec2( Lleft, Ltop ) );
	vec2 edges = step( threshold, delta.xy );

	// Then discard if there is no edge:
	if ( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 ) {
		discard;
	}

	// Calculate right and bottom deltas:
	float Lright = dot( texture2D( u_DiffuseMap, v_Offset[1].xy ).rgb, weights );
	float Lbottom  = dot( texture2D( u_DiffuseMap, v_Offset[1].zw ).rgb, weights );
	delta.zw = abs( L - vec2( Lright, Lbottom ) );

	// Calculate the maximum delta in the direct neighborhood:
	vec2 maxDelta = max( delta.xy, delta.zw );

	// Calculate left-left and top-top deltas:
	float Lleftleft = dot( texture2D( u_DiffuseMap, v_Offset[2].xy ).rgb, weights );
	float Ltoptop = dot( texture2D( u_DiffuseMap, v_Offset[2].zw ).rgb, weights );
	delta.zw = abs( vec2( Lleft, Ltop ) - vec2( Lleftleft, Ltoptop ) );

	// Calculate the final maximum delta:
	maxDelta = max( maxDelta.xy, delta.zw );
	float finalDelta = max( maxDelta.x, maxDelta.y );

	// Local contrast adaptation:
	edges.xy *= step( finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy );

	a_Color = vec4( edges, 0.0, 1.0 );
}
