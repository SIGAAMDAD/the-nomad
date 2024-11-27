layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;

in vec2 v_TexCoords;
in vec4 v_Color;
in vec2 v_WorldPos;
in vec3 v_LightingColor;

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

vec3 CalcSpecular() {
#if defined(USE_SPECULARMAP)
	vec3 upper = texture( u_SpecularMap, v_TexCoords ).rgb;
	vec3 lower = a_Color.rgb;
	vec3 outColor = vec3( 0.0, 0.0, 0.0 );

	if ( upper.r > 0.5 ) {
		outColor.r = ( 1.0 - ( 1.0 - lower.r ) * ( 1.0 - 2.0 * ( upper.r - 0.5 ) ) );
	} else {
		outColor.r = lower.r * ( 2.0 * upper.r );
	}

	if ( upper.g > 0.5 ) {
		outColor.g = ( 1.0 - ( 1.0 - lower.g ) * ( 1.0 - 2.0 * ( upper.g - 0.5 ) ) );
	} else {
		outColor.g = lower.g * ( 2.0 * upper.g );
	}

	if ( upper.b > 0.5 ) {
		outColor.b = ( 1.0 - ( 1.0 - lower.b ) * ( 1.0 - 2.0 * ( upper.b - 0.5 ) ) );
	} else {
		outColor.b = lower.b * ( 2.0 * upper.b );
	}

	return outColor;
#else
	return outColor;
#endif
}

vec3 CalcPointLight( Light light ) {
	vec3 diffuse = a_Color.rgb;
	float dist = distance( v_WorldPos, vec2( light.origin ) );
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
#if !defined(USE_SWITCH)
	if ( u_LightingQuality == QUALITY_NORMAL ) {
		vec3 normal = CalcNormal();
		return ( mix( diffuse, normal, 0.025 ) * attenuation );	
	}
#else
	switch ( u_LightingQuality ) {
	case QUALITY_NORMAL:
		vec3 normal = CalcNormal();
		return ( mix( diffuse, normal, 0.025 ) * attenuation );	
	};
#endif

	vec3 specular = CalcSpecular();
	vec3 normal = CalcNormal();
	diffuse = mix( diffuse, specular, 0.025 );
	diffuse = mix( diffuse, normal, 0.025 );

	return ( diffuse * attenuation );
}

void ApplyLighting() {
	if ( u_LightingQuality == QUALITY_LOW ) {
		a_Color.rgb *= v_LightingColor;
		return;
	}
	for ( int i = 0; i < u_NumLights; i++ ) {
		a_Color.rgb += CalcPointLight( u_LightData[i] );
	}
	a_Color.rgb *= u_AmbientColor;
}

void main() {
	if ( distance( u_ViewOrigin.xy, v_WorldPos.xy ) >= 16.0 ) {
		discard;
	}

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

	float alpha = a_Color.a * v_Color.a;
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
	if ( u_HDR && u_Bloom ) {
		float brightness = dot( a_Color.rgb, vec3( 0.1, 0.1, 0.1 ) );
		if ( brightness > 0.5 ) {
			a_BrightColor = vec4( a_Color.rgb, 1.0 );
		} else {
			a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
		}
	} else {
		a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
	}
	if ( u_GamePaused ) {
		a_Color.rgb = vec3( a_Color.rg * 0.5, 0.5 );
	}
}