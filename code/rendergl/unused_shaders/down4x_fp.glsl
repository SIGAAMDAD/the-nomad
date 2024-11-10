#if !defined(GLSL_LEGACY)
out vec4 a_Color;
#endif

uniform sampler2D u_TextureMap;

uniform vec2 u_InvTexRes;
in vec2 v_TexCoords;

void main()
{
	vec4 color;
	vec2 tc;
	
	tc = v_TexCoords + u_InvTexRes * vec2(-1.5, -1.5);  color  = texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2(-0.5, -1.5);  color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 0.5, -1.5);  color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 1.5, -1.5);  color += texture2D(u_TextureMap, tc);

	tc = v_TexCoords + u_InvTexRes * vec2(-1.5, -0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2(-0.5, -0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 0.5, -0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 1.5, -0.5); color += texture2D(u_TextureMap, tc);

	tc = v_TexCoords + u_InvTexRes * vec2(-1.5,  0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2(-0.5,  0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 0.5,  0.5); color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 1.5,  0.5); color += texture2D(u_TextureMap, tc);

	tc = v_TexCoords + u_InvTexRes * vec2(-1.5,  1.5);  color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2(-0.5,  1.5);  color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 0.5,  1.5);  color += texture2D(u_TextureMap, tc);
	tc = v_TexCoords + u_InvTexRes * vec2( 1.5,  1.5);  color += texture2D(u_TextureMap, tc);
	
	color *= 0.0625;
	
	a_Color = color;
}
