in vec3 a_Position;
in vec2 a_TexCoords;
in uvec2 a_WorldPos;
in vec4 a_Color;
in vec3 a_Normal;
in vec3 a_Tangent;
in vec3 a_Bitangent;

out vec2 v_TexCoords;
out vec3 v_FragPos;
out vec4 v_Color;
out vec3 v_WorldPos;
out vec3 v_Position;
out mat3 v_TBN;

uniform bool u_WorldDrawing;
uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelMatrix;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;
uniform int u_ColorGen;
uniform int u_AlphaGen;

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

#if defined(USE_RGBAGEN)
vec4 CalcColor( vec3 a_Position, vec3 normal )
{
	vec4 color = u_VertColor * a_Color + u_BaseColor;
	
	if (u_ColorGen == CGEN_LIGHTING_DIFFUSE)
	{
		float incoming = clamp(dot(normal, u_ModelLightDir), 0.0, 1.0);

//		color.rgb = clamp(u_DirectedLight * incoming + u_AmbientLight, 0.0, 1.0);
	}
	
	vec3 viewer = u_LocalViewOrigin - a_Position;

	if (u_AlphaGen == AGEN_LIGHTING_SPECULAR)
	{
		vec3 lightDir = normalize(vec3(-960.0, 1980.0, 96.0) - a_Position);
		vec3 reflected = -reflect(lightDir, normal);
		
		color.a = clamp(dot(reflected, normalize(viewer)), 0.0, 1.0);
		color.a *= color.a;
		color.a *= color.a;
	}
	else if (u_AlphaGen == AGEN_PORTAL)
	{
		color.a = clamp(length(viewer) / u_PortalRange, 0.0, 1.0);
	}
	
	return color;
}
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

#if defined(USE_TCGEN)
vec2 GenTexCoords( int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1 )
{
	vec2 tex = texCoords[ a_TileID ];

	if ( TCGen == TCGEN_LIGHTMAP ) {
		tex = texCoords[ a_TileID ].st;
	}
	else if ( TCGen == TCGEN_ENVIRONMENT_MAPPED ) {
		vec3 viewer = normalize( positions[ a_TileID ] - position );
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

vec3 CalcScreenSpaceNormal( vec3 position ) {
	vec3 dx = dFdx( position );
	vec3 dy = dFdy( position );
	return normalize( cross( dx, dy ) );
}

void main() {
	vec3 position = vec3( a_Position.xy, 0.0 );
#if defined(USE_TCGEN)
	vec2 texCoord = GenTexCoords( u_TCGen0, position, vec3( 0.0 ), u_TCGen0Vector0, u_TCGen0Vector1 );
#else
	vec2 texCoord = a_TexCoords;
#endif

#if defined(USE_TCMOD)
	v_TexCoords = ModTexCoords( texCoord, position, u_DiffuseTexMatrix, u_DiffuseTexOffTurb );
#else
	v_TexCoords = texCoord;
#endif
	if ( u_ColorGen == CGEN_VERTEX ) {
		v_Color = vec4( 1.0 );
	} else {
		v_Color = u_VertColor * a_Color + u_BaseColor;
	}
	v_WorldPos = vec3( a_WorldPos.xy, 0.0 );
	v_Position = position;

//	vec3 T = normalize( vec3( u_ModelViewProjection ) )

    gl_Position = u_ModelViewProjection * vec4( a_Position, 1.0 );
}