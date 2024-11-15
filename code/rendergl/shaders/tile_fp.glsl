#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;
#endif

in vec2 v_TexCoords;
in vec4 v_Color;
in vec3 v_WorldPos;
in vec3 v_Position;

uniform float u_GammaAmount;
uniform bool u_GamePaused;
uniform float u_CameraExposure;

uniform bool u_WorldDrawing;
uniform mat4 u_ModelViewProjection;
uniform bool u_HardwareGamma;
uniform bool u_HDR;
uniform bool u_PBR;
uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;
uniform int u_LightingQuality;
uniform bool u_Bloom;
uniform bool u_PostProcess;

#if defined(USE_SHADOWMAP)
TEXTURE2D u_ShadowMap;
#endif

uniform int u_AlphaTest;

TEXTURE2D u_DiffuseMap;
uniform vec3 u_ViewOrigin;
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;

#if defined(USE_NORMALMAP)
TEXTURE2D u_NormalMap;
#endif

#if defined(USE_SPECULARMAP)
TEXTURE2D u_SpecularMap;
#endif

#if defined(USE_PARALLAXMAP)
TEXTURE2D u_LevelsMap;
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

layout( std140, binding = 0 ) readonly buffer u_LightBuffer {
	Light u_LightData[];
};

uniform int u_NumLights;
uniform vec3 u_AmbientColor;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

vec3 CalcNormal() {
#if defined(USE_NORMALMAP)
	vec3 normal = texture( u_NormalMap, v_TexCoords ).rgb;
	normal = normal * 2.0 - 1.0;
	return normalize( normal );
#else
	return vec3( 0.0 );
#endif
}

vec3 CalcScreenSpaceNormal( vec3 position ) {
	vec3 dx = dFdx( position );
	vec3 dy = dFdy( position );
	return normalize( cross( dx, dy ) );
}

vec3 CalcPointLight( Light light ) {
	vec3 diffuse = a_Color.rgb;
	float dist = distance( v_WorldPos, vec3( light.origin, gl_FragCoord.z ) );
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
	if ( u_LightingQuality == 2 ) {
		const vec3 normal = CalcNormal();
		const vec3 lightDir = v_Position - vec3( light.origin, 0.0 );
		const vec3 viewDir = normalize( normalize( u_ViewOrigin ) - v_Position );
		const vec3 reflectDir = reflect( -lightDir, CalcScreenSpaceNormal( v_Position ) );

		float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), 32 );
		vec3 specular = texture( u_SpecularMap, v_TexCoords ).r * spec * light.color.rgb;

		diffuse = mix( diffuse, specular, 0.025 );
		diffuse = mix( diffuse, normal, 0.025 );
	}
	else if ( u_LightingQuality == 1 ) {
		const vec3 normal = CalcNormal();
		diffuse = mix( diffuse, normal, 0.025 );
	}

	diffuse *= attenuation;

	return diffuse;
}

void ApplyLighting() {
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
}

void main() {
//	if ( distance( u_ViewOrigin, v_WorldPos ) > 12.0 ) {
//		discard;
//	}

	// calculate a slight x offset, otherwise we get some black line bleeding
	// going on
	ivec2 texSize = textureSize( u_DiffuseMap, 0 );
	float sOffset = ( 1.0 / ( float( texSize.x ) ) * 0.75 );
	float tOffset = ( 1.0 / ( float( texSize.y ) ) * 0.75 );
	vec2 texCoord = vec2( v_TexCoords.x + sOffset, v_TexCoords.y + tOffset );

	if ( !u_PostProcess ) {
		if ( u_AntiAliasing == AntiAlias_FXAA ) {
			vec2 fragCoord = texCoord * u_ScreenSize;
			a_Color = ApplyFXAA( u_DiffuseMap, fragCoord );
		} else {
			a_Color = sharpenImage( u_DiffuseMap, texCoord );
		}
	} else {
		a_Color = texture( u_DiffuseMap, texCoord );
	}

	const float alpha = a_Color.a * v_Color.a;
	if ( u_AlphaTest == 1 ) {
		if ( alpha == 0.0 ) {
			discard;
		}
	}
	else if ( u_AlphaTest == 2 ) {
		if ( alpha >= 0.5 ) {
			discard;
		}
	}
	else if ( u_AlphaTest == 3 ) {
		if ( alpha < 0.5 ) {
			discard;
		}
	}
	a_Color.a = alpha;

	ApplyLighting();

	// if we have post processing active, don't calculate gamma until the final pass
	if ( u_HDR ) {
		if ( u_Bloom ) {
			// check whether fragment output is higher than threshold, if so output as brightness color
			float brightness = dot( a_Color.rgb, vec3( 0.1, 0.1, 0.1 ) );
			if ( brightness > 0.5 ) {
				a_BrightColor = vec4( a_Color.rgb, 1.0 );
			} else {
				a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
			}
		}
	}
	if ( u_GamePaused ) {
		a_Color.rgb = vec3( a_Color.rg * 0.5, 0.5 );
	}
}