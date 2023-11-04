#if !defined(GLSL_LEGACY)
layout(location = 0) out vec4 a_Color;
#endif

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;

void main() {
    a_Color = v_Color * texture(u_DiffuseMap, v_TexCoord);
}