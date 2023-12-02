in vec3 a_Position;
in vec2 a_TexCoord;
in vec4 a_Color;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec4 v_Color;

uniform mat4 u_ModelViewProjection;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;

#if defined(USE_RGBAGEN)
uniform int u_ColorGen;
uniform int u_AlphaGen;
uniform vec3 u_AmbientLight;
uniform vec3 u_DirectedLight;
#endif

#if defined(USE_RGBAGEN)
vec4 CalcColor(vec3 position, vec3 normal)
{
	vec4 color = u_VertColor * a_Color + u_BaseColor;
	
	if (u_ColorGen == CGEN_LIGHTING_DIFFUSE)
	{
		float incoming = clamp(dot(normal, u_ModelLightDir), 0.0, 1.0);

		color.rgb = clamp(u_DirectedLight * incoming + u_AmbientLight, 0.0, 1.0);
	}
	
	vec3 viewer = u_LocalViewOrigin - position;

	if (u_AlphaGen == AGEN_LIGHTING_SPECULAR)
	{
		vec3 lightDir = normalize(vec3(-960.0, 1980.0, 96.0) - position);
		vec3 reflected = -reflect(lightDir, normal);
		
		color.a = clamp(dot(reflected, normalize(viewer)), 0.0, 1.0);
		color.a *= color.a;
		color.a *= color.a;
	}
	else if (u_AlphaGen == AGEN_PORTAL)
	{
		color.a = clamp(length(viewer) / u_PortalRange, 0.0, 1.0);
	}
	
	return color;
}
#endif

void main()
{
#if defined(USE_RGBAGEN)
    v_Color = CalcColor(a_Position, a_Normal);
#else
//    v_Color = u_VertColor * a_Color + u_BaseColor;
#endif
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_FragPos = vec4(u_ModelViewProjection * vec4(a_Position, 1.0)).xyz;

    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0);
}