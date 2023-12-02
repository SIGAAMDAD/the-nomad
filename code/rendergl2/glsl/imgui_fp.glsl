#if !defined(GLSL_LEGACY)
layout(location = 0) out vec4 a_Color;
#endif

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;

void main() {
    a_Color = v_Color * texture(u_DiffuseMap, v_TexCoord);
    a_Color.rgb = pow(a_Color.rgb, vec3( 1.0 / u_GammaAmount ));
}