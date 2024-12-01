#if defined(USE_MULTIATTRIB)
layout( location = 0 ) out vec4 a_Color;
#else
out vec4 a_Color;
#endif

TEXTURE2D u_DiffuseMap;
uniform vec4 u_Color;
uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;
uniform float u_GammaAmount;
uniform bool u_FinalPass;

in vec2 v_TexCoords;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

void main() {
	a_Color = texture( u_DiffuseMap, v_TexCoords );
}
