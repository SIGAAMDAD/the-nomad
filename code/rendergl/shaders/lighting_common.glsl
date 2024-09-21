uniform bool u_InLevel;
uniform sampler2D u_DiffuseMap;

#ifdef FRAGMENT_SHADER
#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif
#endif

#if LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1

#ifdef VERTEX_SHADER
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

layout( std140, binding = 0 ) uniform u_LightBuffer {
	Light u_LightData[ MAX_MAP_LIGHTS ];
};

uniform int u_NumLights;
uniform vec3 u_AmbientColor;

out vec3 v_LightColor;
#elif defined(FRAGMENT_SHADER)
in vec3 v_LightColor;
#endif

#elif LIGHTING_QUALITY == 2

#ifdef FRAGMENT_SHADER
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

layout( std140, binding = 0 ) uniform u_LightBuffer {
	Light u_LightData[ MAX_MAP_LIGHTS ];
};

uniform int u_NumLights;
uniform vec3 u_AmbientColor;
#endif

#endif

#if LIGHTING_QUALITY == 2
#ifdef FRAGMENT_SHADER
float CalcLightAttenuation(float point, float normDist)
{
	// zero light at 1.0, approximating q3 style
	// also don't attenuate directional light
	float attenuation = (0.5 * normDist - 1.5) * point + 1.0;

	// clamp attenuation
	#if defined(NO_LIGHT_CLAMP)
	attenuation = max(attenuation, 0.0);
	#else
	attenuation = clamp(attenuation, 0.0, 1.0);
	#endif

	return attenuation;
}

vec3 CalcDiffuse( vec3 diffuseAlbedo, float NH, float EH, float roughness )
{
#if defined(USE_BURLEY)
	// modified from https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
	float fd90 = -0.5 + EH * EH * roughness;
	float burley = 1.0 + fd90 * 0.04 / NH;
	burley *= burley;
	return diffuseAlbedo * burley;
#else
	return diffuseAlbedo;
#endif
}

void CalcNormal() {
#if defined(USE_NORMALMAP)
	vec3 normal = texture2D( u_NormalMap, v_TexCoords ).rgb;
	normal = normalize( normal * 2.0 - 1.0 );
	a_Color.rgb *= normal * 0.5 + 0.5;
#endif
}
#endif
#endif

//
// CalcPointLight: don't modify, straight from Valden
//
#if LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1
#ifdef VERTEX_SHADER
vec3 CalcPointLight( Light light ) {
	vec3 diffuse = v_LightColor;
	float dist = distance( v_WorldPos, vec3( light.origin, v_WorldPos.z ) );
	float diff = 0.0;
	float range = light.range;
	if ( dist <= light.range ) {
		diff = 1.0 - abs( dist / range );
	}
	diff += light.brightness;
	diffuse = min( diff * ( diffuse + vec3( light.color ) ), diffuse );

	range = light.range + light.brightness;
	float attenuation = ( light.constant + light.linear * range
		+ light.quadratic * ( range * range ) );

	diffuse *= attenuation;

	return diffuse;
}
#endif
#elif LIGHTING_QUALITY == 2
#ifdef FRAGMENT_SHADER
vec3 CalcPointLight( Light light ) {
	vec3 diffuse = a_Color.rgb;
	float dist = distance( v_WorldPos, vec3( light.origin, v_WorldPos.z ) );
	float diff = 0.0;
	float range = light.range;
	if ( dist <= light.range ) {
		diff = 1.0 - abs( dist / range );
	}
	diff += light.brightness;
	diffuse = min( diff * ( diffuse + vec3( light.color ) ), diffuse );

	range = light.range + light.brightness;
	float attenuation = ( light.constant + light.linear * range
		+ light.quadratic * ( range * range ) );

	vec3 lightDir = vec3( 0.0 );
	vec3 viewDir = normalize( v_WorldPos - vec3( light.origin, 0.0 ) );
	vec3 halfwayDir = normalize( lightDir + viewDir );

	vec3 reflectDir = reflect( -lightDir, v_WorldPos );
	float spec = pow( max( dot( v_WorldPos, reflectDir ), 0.0 ), 1.0 );
#if defined(USE_SPECULARMAP)
	vec3 specular = spec * texture2D( u_SpecularMap, v_TexCoords ).rgb;
#else
	vec3 specular = vec3( 0.0 );
#endif

	diffuse *= attenuation;

	return diffuse;
}
#endif
#endif

void ApplyLighting() {
	if ( !u_InLevel ) {
		// if we're not in a level, don't apply level lighting
//		return;
	}
#if LIGHTING_QUALITY == 0 || LIGHTING_QUALITY == 1
#ifdef VERTEX_SHADER
	v_LightColor = vec3( 1.0 );
	for ( int i = 0; i < u_NumLights; i++ ) {
		switch ( u_LightData[i].type ) {
		case POINT_LIGHT:
			v_LightColor += CalcPointLight( u_LightData[i] );
			break;
		case DIRECTION_LIGHT:
			break;
		};
	}
	v_LightColor *= u_AmbientColor;
#endif
#ifdef FRAGMENT_SHADER
	a_Color.rgb *= v_LightColor;
#endif
#elif LIGHTING_QUALITY == 2
#ifdef FRAGMENT_SHADER
	CalcNormal();
	for ( int i = 0; i < u_NumLights; i++ ) {
		switch ( u_LightData[i].type ) {
		case POINT_LIGHT:
			a_Color.rgb += CalcPointLight( u_LightData[i] );
			break;
		case DIRECTION_LIGHT:
			break;
		};
	}
	a_Color.rgb *= u_AmbientColor;
#endif
#endif
}