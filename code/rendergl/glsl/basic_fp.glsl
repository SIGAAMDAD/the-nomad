#if !defined(GLSL_LEGACY)
out vec4 a_Color;
#endif

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;

uniform vec4 u_AmbientColor;
uniform float u_AmbientIntensity;
#endif

#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if defined(USE_SHADOWMAP)
uniform sampler2D u_ShadowMap;
#endif

uniform int u_AlphaTest;

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

vec3 CalcDiffuse(vec3 diffuseAlbedo, float NH, float EH, float roughness)
{
#if defined(USE_BURLEY)
	// modified from https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
	float fd90 = -0.5 + EH * EH * roughness;
	float burley = 1.0 + fd90 * 0.04 / NH;
	burley *= burley;
	return diffuseAlbedo * burley;
#else
	return diffuseAlbedo;
#endif
}

#if 0
void applyLighting() {
    vec3 ambient = u_AmbientColor + u_AmbientIntensity;

    for (int i = 0; i < u_numLights; i++) {
        float diffuse = 0.0;
        mat4 model = mat4(1.0);

        //
        // using world position will give the lighting a more
        // retro and pixel-game (8-bit) style look because
        // individual tiles will be lit up
        //

        float dist = distance(lights[i].origin, v_WorldPos);
        vec3 color = lights[i].color + lights[i].intensity.x;
        float range = lights[i].intensity.y;

        if (dist <= range) {
            diffuse = 1.0 - abs(dist / lights[i].intensity.x);
        }

        a_Color *= vec4(min(a_Color.rgb * ((lights[i].color * diffuse) + u_AmbientColor), a_Color.rgb), 1.0);
    }
}
#endif

void AmbientLight() {
//    vec3 color = u_AmbientColor + u_AmbientIntensity;
}

void main() {
    vec4 color = v_Color;
    color = vec4(0.0);
    a_Color = texture2D( u_DiffuseMap, v_TexCoord.st ) * color;
}
