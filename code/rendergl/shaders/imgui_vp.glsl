in vec3 a_Position;
in vec2 a_TexCoords;
in vec4 a_Color;

uniform mat4 u_ModelViewProjection;

out vec2 v_TexCoords;
out vec4 v_Color;

#if defined(USE_TCGEN)
uniform vec4 u_DiffuseTexMatrix;
uniform vec4 u_DiffuseTexOffTurb;
#endif

#if defined(USE_TCGEN)
uniform int u_TCGen0;
uniform vec3 u_TCGen0Vector0;
uniform vec3 u_TCGen0Vector1;
uniform vec3 u_WorldPos;
#endif


#if defined(USE_TCMOD)
vec2 ModTexCoords( vec2 st, vec3 position, vec4 texMatrix, vec4 offTurb )
{
	float amplitude = offTurb.z;
	float phase = offTurb.w * 2.0 * M_PI;
	vec2 st2;

	st2.x = st.x * texMatrix.x + ( st.y * texMatrix.z + offTurb.x );
	st2.y = st.x * texMatrix.y + ( st.y * texMatrix.w + offTurb.y );

	vec2 offsetPos = vec2( position.x + position.z, position.y );

	vec2 texOffset = sin( offsetPos * ( 2.0 * M_PI / 1024.0 ) + vec2( phase ) );

	return st2 + texOffset * amplitude;	
}
#endif

float CalcLightAttenuation( float point, float normDist )
{
	// zero light at 1.0, approximating q3 style
	// also don't attenuate directional light
	float attenuation = ( 0.5 * normDist - 1.5 ) * point + 1.0;

	// clamp attenuation
#if defined(NO_LIGHT_CLAMP)
	attenuation = max( attenuation, 0.0 );
#else
	attenuation = clamp( attenuation, 0.0, 1.0 );
#endif

	return attenuation;
}

#if defined(USE_TCGEN)
vec2 GenTexCoords( int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1 )
{
	vec2 tex = a_TexCoords;

	if ( TCGen == TCGEN_LIGHTMAP ) {
		tex = a_TexCoords.st;
	}
	else if ( TCGen == TCGEN_ENVIRONMENT_MAPPED ) {
		vec3 viewer = normalize( vec3( 0.0 ) - position );
		vec2 ref = reflect( viewer, normal ).yz;
		tex.s = ref.x * -0.5 + 0.5;
		tex.t = ref.y *  0.5 + 0.5;
	}
	else if ( TCGen == TCGEN_VECTOR ) {
		tex = vec2( dot( position, TCGenVector0 ), dot( position, TCGenVector1 ) );
	}

	return tex;
}
#endif

void main()
{

#if defined(USE_TCGEN)
	vec2 texCoords = GenTexCoords( u_TCGen0, a_Position, vec3( 0.0 ), u_TCGen0Vector0, u_TCGen0Vector1 );
#else
	vec2 texCoords = a_TexCoords;
#endif

#if defined(USE_TCMOD)
	v_TexCoords = ModTexCoords( texCoords, a_Position, u_DiffuseTexMatrix, u_DiffuseTexOffTurb );
#else
	v_TexCoords = texCoords;
#endif
    v_Color = a_Color;
    gl_Position = u_ModelViewProjection * vec4( a_Position.xy, 0, 1 );
}