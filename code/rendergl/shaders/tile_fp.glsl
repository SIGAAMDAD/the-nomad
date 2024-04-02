#if !defined(GLSL_LEGACY)
out vec4 a_Color;
#endif

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec4 v_Color;
in vec3 v_WorldPos;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;

#if defined(USE_LIGHT)
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;
uniform uint u_NumLights;
#endif

#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

uniform int u_AlphaTest;

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

vec3 CalcDiffuse(vec3 diffuseAlbedo, float NH, float EH, float roughness)
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

#if defined(USE_NORMALMAP)
void CalcNormal() {
    vec3 normal = texture2D( u_NormalMap, v_TexCoords ).rgb;
    normal = normalize( normal * 2.0 - 1.0 );
    a_Color.rgb *= normal * 0.5 + 0.5;
}
#endif

void applyLighting() {
#if defined(USE_NORMALMAP)
    CalcNormal();
#endif
#if defined(USE_SPECULARMAP)
    a_Color.rgb += texture2D( u_SpecularMap, v_TexCoords ).rgb;
#endif
#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
    if ( u_NumLights == uint( 0 ) )
#endif
	{
        a_Color.rgb += texture2D( u_DiffuseMap, v_TexCoords ).rgb;
    }

#if defined(USE_LIGHT)
    a_Color.rgb *= u_AmbientColor;
#endif
}

void main() {
    a_Color = texture2D( u_DiffuseMap, v_TexCoords );

    applyLighting();

#if defined(USE_BLOOM) && !defined(USE_FAST_LIGHT)
	const float brightness = dot( a_Color.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
	if ( brightness > 1.0 ) {
		a_BrightColor = vec4( a_Color.rgb, 1.0 );
	} else {
		a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
#endif
#if defined(USE_HDR)
	// reinhard tone mapping
	a_Color.rgb /= ( a_Color.rgb + vec3( 1.0 ) );
#endif
    a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
}
