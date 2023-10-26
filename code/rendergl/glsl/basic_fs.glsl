#if !defined(GLSL_LEGACY)
out vec4 out_Color;
#endif

in vec2 v_TexCoords;
in vec3 v_Position;
in vec3 v_Color;
in float v_Alpha;

uniform sampler2D u_DiffuseMap;

void main()
{
    out_Color = texture(u_DiffuseMap, v_TexCoords);
}