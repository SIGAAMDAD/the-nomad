in vec4 a_TexCoords;
#if defined(USE_LIGHTMAP) || defined(USE_TCGEN)
in vec4 a_LightCoords;
#endif
in vec4 a_Color;

in vec3 a_Position;
in vec3 a_Normal;
in vec4 a_Tangent;

#if defined(USE_LIGHT) && !defined(USE_LIGHT_VECTOR)
//in vec3 a_LightDirection;
#endif

#if defined(USE_DELUXEMAP)
uniform vec4   u_EnableTextures; // x = normal, y = deluxe, z = specular, w = cube
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec3   u_ViewOrigin;
#endif

#if defined(USE_TCGEN)
uniform int    u_TCGen0;
uniform vec3   u_TCGen0Vector0;
uniform vec3   u_TCGen0Vector1;
uniform vec3   u_LocalViewOrigin;
#endif

#if defined(USE_TCMOD)
uniform vec4   u_DiffuseTexMatrix;
uniform vec4   u_DiffuseTexOffTurb;
#endif

uniform mat4   u_ModelViewProjectionMatrix;
uniform vec4   u_BaseColor;
uniform vec4   u_VertColor;

#if defined(USE_MODELMATRIX)
uniform mat4   u_ModelMatrix;
#endif

#if defined(USE_LIGHT_VECTOR)
uniform vec4   u_LightOrigin;
uniform float  u_LightRadius;
uniform vec3   u_DirectedLight;
uniform vec3   u_AmbientLight;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
uniform vec4  u_PrimaryLightOrigin;
uniform float u_PrimaryLightRadius;
#endif

out vec4   v_TexCoords;

out vec4   v_Color;
#if defined(USE_LIGHT_VECTOR) && !defined(USE_FAST_LIGHT)
out vec4   v_ColorAmbient;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
out vec4   v_Normal;
out vec4   v_Tangent;
out vec4   v_Bitangent;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
out vec4   v_LightDir;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
out vec4   v_PrimaryLightDir;
#endif

#if defined(USE_TCGEN)
vec2 GenTexCoords(int TCGen, vec3 position, vec3 normal, vec3 TCGenVector0, vec3 TCGenVector1)
{
	vec2 tex = a_TexCoords.st;

	if (TCGen == TCGEN_LIGHTMAP)
	{
		tex = a_LightCoords.st;
	}
	else if (TCGen == TCGEN_ENVIRONMENT_MAPPED)
	{
		vec3 viewer = normalize(u_LocalViewOrigin - position);
		vec2 ref = reflect(viewer, normal).yz;
		tex.s = ref.x * -0.5 + 0.5;
		tex.t = ref.y *  0.5 + 0.5;
	}
	else if (TCGen == TCGEN_VECTOR)
	{
		tex = vec2(dot(position, TCGenVector0), dot(position, TCGenVector1));
	}

	return tex;
}
#endif

#if defined(USE_TCMOD)
vec2 ModTexCoords(vec2 st, vec3 position, vec4 texMatrix, vec4 offTurb)
{
	float amplitude = offTurb.z;
	float phase = offTurb.w * 2.0 * M_PI;
	vec2 st2;
	st2.x = st.x * texMatrix.x + (st.y * texMatrix.z + offTurb.x);
	st2.y = st.x * texMatrix.y + (st.y * texMatrix.w + offTurb.y);

	vec2 offsetPos = vec2(position.x + position.z, position.y);

	vec2 texOffset = sin(offsetPos * (2.0 * M_PI / 1024.0) + vec2(phase));

	return st2 + texOffset * amplitude;	
}
#endif


float CalcLightAttenuation(float point, float normDist)
{
	// zero light at 1.0, approximating q3 style
	// also don't attenuate directional light
	float attenuation = (0.5 * normDist - 1.5) * point + 1.0;

	// clamp attenuation
	#if defined(NO_LIGHT_CLAMP)
	attenuation = max(attenuation, 0.0);
	#else
	attenuation = clamp(attenuation, 0.0, 1.0);
	#endif

	return attenuation;
}


void main()
{
	vec3 position  = a_Position;
	vec3 normal    = a_Normal;
#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 tangent   = a_Tangent.xyz;
#endif

#if defined(USE_TCGEN)
	vec2 texCoords = GenTexCoords(u_TCGen0, position, normal, u_TCGen0Vector0, u_TCGen0Vector1);
#else
	vec2 texCoords = a_TexCoords.st;
#endif

#if defined(USE_TCMOD)
	v_TexCoords.xy = ModTexCoords(texCoords, position, u_DiffuseTexMatrix, u_DiffuseTexOffTurb);
#else
	v_TexCoords.xy = texCoords;
#endif

	gl_Position = u_ModelViewProjectionMatrix * vec4(position, 1.0);

#if defined(USE_MODELMATRIX)
	position  = (u_ModelMatrix * vec4(position, 1.0)).xyz;
	normal    = (u_ModelMatrix * vec4(normal,   0.0)).xyz;
  #if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	tangent   = (u_ModelMatrix * vec4(tangent,  0.0)).xyz;
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 bitangent = cross(normal, tangent) * a_Tangent.w;
#endif

#if defined(USE_LIGHT_VECTOR)
	vec3 L = u_LightOrigin.xyz - (position * u_LightOrigin.w);
#elif defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
//	vec3 L = a_LightDirection;
	vec3 L = vec3( 0.0 );
  #if defined(USE_MODELMATRIX)
	L = (u_ModelMatrix * vec4(L, 0.0)).xyz;
  #endif
#endif

#if defined(USE_LIGHTMAP)
	v_TexCoords.zw = a_LightCoords.st;
#endif

	v_Color = u_VertColor * a_Color + u_BaseColor;

#if defined(USE_LIGHT_VECTOR)
  #if defined(USE_FAST_LIGHT)
	float sqrLightDist = dot(L, L);
	float NL = clamp(dot(normalize(normal), L) / sqrt(sqrLightDist), 0.0, 1.0);
	float attenuation = CalcLightAttenuation(u_LightOrigin.w, u_LightRadius * u_LightRadius / sqrLightDist);

	v_Color.rgb *= u_DirectedLight * (attenuation * NL) + u_AmbientLight;
  #else
	v_ColorAmbient.rgb = u_AmbientLight * v_Color.rgb;
	v_Color.rgb *= u_DirectedLight;
    #if defined(USE_PBR)
	v_ColorAmbient.rgb *= v_ColorAmbient.rgb;
    #endif
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT) && defined(USE_PBR)
	v_Color.rgb *= v_Color.rgb;
#endif

#if defined(USE_PRIMARY_LIGHT) || defined(USE_SHADOWMAP)
	v_PrimaryLightDir.xyz = u_PrimaryLightOrigin.xyz - (position * u_PrimaryLightOrigin.w);
	v_PrimaryLightDir.w = u_PrimaryLightRadius * u_PrimaryLightRadius;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
  #if defined(USE_LIGHT_VECTOR)
	v_LightDir = vec4(L, u_LightRadius * u_LightRadius);
  #else
	v_LightDir = vec4(L, 0.0);
  #endif
  #if defined(USE_DELUXEMAP)
	v_LightDir -= u_EnableTextures.y * v_LightDir;
  #endif
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
	vec3 viewDir = u_ViewOrigin - position;
	// store view direction in tangent space to save on outs
	v_Normal    = vec4(normal,    viewDir.x);
	v_Tangent   = vec4(tangent,   viewDir.y);
	v_Bitangent = vec4(bitangent, viewDir.z);
#endif
}
