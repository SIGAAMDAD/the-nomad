#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#endif

uniform sampler2D u_DiffuseMap;
uniform vec4 u_Color;
uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;
uniform float u_GammaAmount;
uniform bool u_FinalPass;

in vec2 v_TexCoords;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

void main() {
	if ( u_FinalPass ) {
		if ( u_AntiAliasing == AntiAlias_FXAA ) {
			a_Color = ApplyFXAA( u_DiffuseMap, v_TexCoords );
		}
		a_Color *= sharpenImage( u_DiffuseMap, v_TexCoords );
	} else {
		a_Color = texture2D( u_DiffuseMap, v_TexCoords );
	}
}
