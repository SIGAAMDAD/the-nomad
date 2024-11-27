#if defined(USE_MULTIATTRIB)
layout( location = 0 ) out vec4 a_Color;
#else
out vec4 a_Color;
#endif

in vec2 v_TexCoords;

TEXTURE2D u_DiffuseMap;
#if defined(USE_MULTIATTRIB)
TEXTURE2D u_BrightMap;
#endif

uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;
uniform bool u_Bloom;
uniform bool u_HDR;
uniform float u_GammaAmount;
uniform float u_CameraExposure;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

void main() {
	vec3 color;

	if ( u_AntiAliasing == AntiAlias_FXAA ) {
		vec2 fragCoord = v_TexCoords * u_ScreenSize;
		a_Color = ApplyFXAA( u_DiffuseMap, fragCoord );
	} else {
		a_Color = texture( u_DiffuseMap, v_TexCoords );
	}
	a_Color.rgb *= sharpenImage( u_DiffuseMap, v_TexCoords ).rgb;

#if defined(USE_MULTIATTRIB)
	if ( u_Bloom ) {
		vec3 bloomColor = texture( u_BrightMap, v_TexCoords ).rgb;
		a_Color.rgb += bloomColor;
	}
#endif

	// exposure tone mapping
	a_Color.rgb = vec3( 1.0 ) - exp( -a_Color.rgb * u_CameraExposure );

	// apply final gamma
	a_Color = vec4( pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) ), 1.0 );
}