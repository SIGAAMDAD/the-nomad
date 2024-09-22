#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;
#endif

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_WorldPos;
in vec4 v_Color;

uniform float u_GammaAmount;
uniform bool u_GamePaused;
uniform bool u_HardwareGamma;
uniform int u_AntiAliasing;

uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;

uniform float u_CameraExposure;
uniform bool u_HDR;
uniform bool u_PBR;
uniform bool u_Bloom;
uniform int u_ToneMap;


#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

uniform int u_AlphaTest;

#include "lighting_common.glsl"
#include "image_sharpen.glsl"

void texcoords( vec2 fragCoord, vec2 resolution, out vec2 v_rgbNW, out vec2 v_rgbNE, out vec2 v_rgbSW,
	out vec2 v_rgbSE, out vec2 v_rgbM )
{
	vec2 inverseVP = 1.0 / resolution.xy;
	v_rgbNW = ( fragCoord + vec2( -1.0, -1.0 ) ) * inverseVP;
	v_rgbNE = ( fragCoord + vec2( 1.0, -1.0 ) ) * inverseVP;
	v_rgbSW = ( fragCoord + vec2( -1.0, 1.0 ) ) * inverseVP;
	v_rgbSE = ( fragCoord + vec2( 1.0, 1.0 ) ) * inverseVP;
	v_rgbM = vec2( fragCoord * inverseVP );
}

#ifndef FXAA_REDUCE_MIN
	#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
	#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
	#define FXAA_SPAN_MAX     8.0
#endif

//optimized version for mobile, where dependent 
//texture reads can be a bottleneck
vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
			vec2 v_rgbNW, vec2 v_rgbNE, 
			vec2 v_rgbSW, vec2 v_rgbSE, 
			vec2 v_rgbM) {
	vec4 color;
	vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
	vec3 rgbNW = texture2D(tex, v_rgbNW).xyz;
	vec3 rgbNE = texture2D(tex, v_rgbNE).xyz;
	vec3 rgbSW = texture2D(tex, v_rgbSW).xyz;
	vec3 rgbSE = texture2D(tex, v_rgbSE).xyz;
	vec4 texColor = texture2D(tex, v_rgbM);
	vec3 rgbM  = texColor.xyz;
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
						  (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	
	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
			  max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
			  dir * rcpDirMin)) * inverseVP;
	
	vec3 rgbA = 0.5 * (
		texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
		texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (
		texture2D(tex, fragCoord * inverseVP + dir * -0.5).xyz +
		texture2D(tex, fragCoord * inverseVP + dir * 0.5).xyz);

	float lumaB = dot(rgbB, luma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
		color = vec4(rgbA, texColor.a);
	else
		color = vec4(rgbB, texColor.a);
	return color;
}

vec4 applyFXAA( sampler2D tex, vec2 fragCoord, vec2 resolution ) {
	vec2 v_rgbNW;
	vec2 v_rgbNE;
	vec2 v_rgbSW;
	vec2 v_rgbSE;
	vec2 v_rgbM;

	texcoords( fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );

	return fxaa( tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );
}

vec3 blur( vec3 color )
{
	float weight[5] = float[]( 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 );
	vec2 tex_offset = 1.0 / textureSize( u_DiffuseMap, 0 );
	vec3 result = color * weight[0];

	for ( int h = 0; h < 2; h++ ) {
		bool horizontal = h == 1;
		if ( horizontal ) {
			for ( int i = 1; i < 5; ++i ) {
				result += texture2D( u_DiffuseMap, v_TexCoords + vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[i];
				result += texture2D( u_DiffuseMap, v_TexCoords - vec2( tex_offset.x * i, 0.0 ) ).rgb * weight[i];
			}
		}
		else {
			for( int i = 1; i < 5; ++i ) {
				result += texture2D( u_DiffuseMap, v_TexCoords + vec2( 0.0, tex_offset.y * i ) ).rgb * weight[i];
				result += texture2D( u_DiffuseMap, v_TexCoords - vec2( 0.0, tex_offset.y * i ) ).rgb * weight[i];
			}
		}
	}

	return result;
}

void main() {
	// calculate a slight x offset, otherwise we get some black line bleeding
	// going on
	ivec2 texSize = textureSize( u_DiffuseMap, 0 );
	float sOffset = ( 1.0 / ( float( texSize.x ) ) * 0.75 );
	float tOffset = ( 1.0 / ( float( texSize.y ) ) * 0.75 );
	vec2 texCoord = vec2( v_TexCoords.x + sOffset, v_TexCoords.y + tOffset );

	if ( u_AntiAliasing == AntiAlias_FXAA ) {
		vec2 fragCoord = texCoord * u_ScreenSize;
		a_Color = applyFXAA( u_DiffuseMap, fragCoord, u_ScreenSize );
	} else {
		a_Color = sharpenImage( u_DiffuseMap, texCoord );
	}
	if ( a_Color.a == 0.0 ) {
		discard;
	}

	ApplyLighting();

#ifdef USE_HDR
#if TONEMAP_TYPE == ToneMap_Reinhard
	// reinhard tone mapping
	a_Color.rgb = a_Color.rgb / ( a_Color.rgb + vec3( 1.0 ) );
#elif TONEMAP_TYPE == ToneMap_Exposure
	// exposure tone mapping
	a_Color.rgb = vec3( 1.0 ) - exp( -a_Color.rgb * u_CameraExposure );
#endif
#endif

#ifdef USE_BLOOM
	// check whether fragment output is higher than threshold, if so output as brightness color
	float brightness = dot( a_Color.rgb, vec3( 0.1, 0.1, 0.1 ) );
	if ( brightness > 0.5 ) {
		a_BrightColor = vec4( a_Color.rgb, 1.0 );
	} else {
		a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
#endif
	a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
	a_Color.rgb *= v_Color.rgb;

	if ( u_GamePaused ) {
		a_Color.rgb = vec3( a_Color.rg * 0.2, 0.5 );
	}
}