#if defined(USE_MULTIATTRIB)
layout( location = 0 ) out vec4 a_Color;
layout( location = 1 ) out vec4 a_BrightColor;
#else
out vec4 a_Color;
#endif

in vec2 v_TexCoords;
in vec4 v_Color;

TEXTURE2D u_DiffuseMap;

uniform float u_GammaAmount;
uniform bool u_GamePaused;
uniform bool u_HardwareGamma;
uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;

uniform bool u_WorldDrawing;
uniform float u_CameraExposure;
uniform bool u_HDR;
uniform bool u_PBR;
uniform bool u_Bloom;
uniform int u_ToneMap;

uniform int u_AlphaTest;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

void main() {
	if ( u_AntiAliasing == AntiAlias_FXAA ) {
		vec2 fragCoord = v_TexCoords * u_ScreenSize;
		a_Color = ApplyFXAA( u_DiffuseMap, v_TexCoords );
	} else {
		a_Color = sharpenImage( u_DiffuseMap, v_TexCoords );
	}
	if ( a_Color.a == 0.0 ) {
		discard;
	}

#if defined(USE_MULTIATTRIB)
	if ( u_Bloom && u_HDR ) {
		// check whether fragment output is higher than threshold, if so output as brightness color
		float brightness = dot( a_Color.rgb, vec3( 0.1, 0.1, 0.1 ) );
		if ( brightness > 0.5 ) {
			a_BrightColor = vec4( a_Color.rgb, 1.0 );
		} else {
			a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
		}
	}
	else
#endif
	{
		a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
	}
	a_Color.rgb *= v_Color.rgb;

	if ( u_GamePaused ) {
		a_Color.rgb = vec3( a_Color.rg * 0.2, 0.5 );
	}
}