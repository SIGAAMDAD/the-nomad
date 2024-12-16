in vec2 a_Position;
in vec2 a_TexCoords;
in vec2 a_WorldPos;
in vec4 a_Color;

out vec2 v_TexCoords;
out vec4 v_Color;
out vec2 v_WorldPos;
out vec3 v_LightingColor;
out vec2 v_Position;

uniform mat4 u_ModelViewProjection;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;
uniform int u_ColorGen;

uniform int u_LightingQuality;
uniform vec3 u_AmbientColor;
uniform int u_NumLights;

TEXTURE2D u_DiffuseMap;

#if defined(USE_RGBAGEN)
uniform int u_ColorGen;
uniform int u_AlphaGen;
uniform vec3 u_DirectedLight;
#endif

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

struct Light {
	vec4 color;
	uvec2 origin;
	float brightness;
	float range;
	float linear;
	float quadratic;
	float constant;
	int type;
};

#if defined(EXPLICIT_BUFFER_LOCATIONS)
layout( std140, binding = 0 ) readonly buffer u_LightBuffer {
	Light u_LightData[];
};
#else
layout( std140 ) uniform u_LightBuffer {
	Light u_LightData[ MAX_LIGHTS ];
};
#endif

vec3 CalcPointLight( Light light ) {
	vec3 diffuse = v_LightingColor.rgb;
	float dist = distance( v_WorldPos, light.origin );
	float diff = 0.0;
	float range = light.range;
	float attenuation = 1.0;

	if ( dist <= light.range ) {
		diff = 1.0 - abs( dist / light.range );
	}
	diff += light.brightness;
	diffuse = min( diff * ( diffuse + vec3( light.color ) ), diffuse );

	range = light.range * light.brightness;

	attenuation = ( light.constant + light.linear + light.quadratic * ( light.range * light.range ) );
	
	return ( diffuse * attenuation );
}

void main() {
	if ( u_ColorGen == CGEN_VERTEX ) {
		v_Color = vec4( 1.0 );
	} else {
		v_Color = u_VertColor * a_Color + u_BaseColor;
	}
	v_WorldPos = a_WorldPos;
	
#if defined(USE_TCGEN)
	vec2 texCoords = GenTexCoords( u_TCGen0, vec3( a_Position.xy, 0.0 ), vec3( 0.0 ), u_TCGen0Vector0, u_TCGen0Vector1 );
#else
	vec2 texCoords = a_TexCoords;
#endif

#if defined(USE_TCMOD)
	v_TexCoords = ModTexCoords( texCoords, vec3( a_Position.xy, 0.0 ), u_DiffuseTexMatrix, u_DiffuseTexOffTurb );
#else
	v_TexCoords = texCoords;
#endif

	if ( u_LightingQuality == QUALITY_LOW ) {
		v_LightingColor = vec3( 1.0 );
		for ( int i = 0; i < u_NumLights; i++ ) {
			v_LightingColor += CalcPointLight( u_LightData[ i ] );
		}
		v_LightingColor *= u_AmbientColor;
	}

	gl_Position = u_ModelViewProjection * vec4( a_Position, 0.0, 1.0 );
	v_Position.xy = gl_Position.xy;
}