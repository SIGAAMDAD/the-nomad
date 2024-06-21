#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#if defined(USE_BLOOM) && defined(USE_HDR)
layout( location = 1 ) out vec4 a_BrightColor;
#endif
#endif

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_WorldPos;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;
uniform int u_NumLights;

uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;

struct Light {
    vec4 color;
    uvec2 origin;
    float brightness;
    float range;
    float linear;
    float quadratic;
    float constant;
    int type;
};
layout( std140, binding = 0 ) uniform u_LightBuffer {
    Light u_LightData[MAX_MAP_LIGHTS];
};
#endif

#if defined(USE_HDR) && defined(USE_EXPOSURE_TONE_MAPPING)
uniform float u_Exposure;
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

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
//
// CalcPointLight: don't modify, straight from Valden
//
vec3 CalcPointLight( Light light ) {
    vec3 diffuse = a_Color.rgb;
    float dist = distance( v_WorldPos, vec3( light.origin, v_WorldPos.z ) );
    float diff = 0.0;
    float range = light.range;
    if ( dist <= light.range ) {
        diff = 1.0 - abs( dist / range );
    }
    diff += light.brightness;
    diffuse = min( diff * ( diffuse + vec3( light.color ) ), diffuse );

    vec3 lightDir = vec3( 0.0 );
    vec3 viewDir = normalize( v_WorldPos - vec3( light.origin, 0.0 ) );
    vec3 halfwayDir = normalize( lightDir + viewDir );

    vec3 reflectDir = reflect( -lightDir, v_WorldPos );
    float spec = pow( max( dot( v_WorldPos, reflectDir ), 0.0 ), 1.0 );

#if defined(USE_SPECULARMAP)
    vec3 specular = spec * texture( u_SpecularMap, v_TexCoords ).rgb;
#else
    vec3 specular = vec3( 0.0 );
#endif

    range = light.range + light.brightness;
    float attenuation = ( light.constant + light.linear * range
        + light.quadratic * ( range * range ) );
    
    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular;
}
#endif

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

void CalcNormal() {
#if defined(USE_NORMALMAP)
    vec3 normal = texture( u_NormalMap, v_TexCoords ).rgb;
    normal = normalize( normal * 2.0 - 1.0 );
    a_Color.rgb *= normal * 0.5 + 0.5;
#endif
}

void ApplyLighting() {
    CalcNormal();
#if defined(USE_SPECULARMAP)
    if ( u_NumLights == 0 ) {
        a_Color.rgb += texture( u_SpecularMap, v_TexCoords ).rgb;
    }
#endif
#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
    for ( int i = 0; i < u_NumLights; i++ ) {
        switch ( u_LightData[i].type ) {
        case POINT_LIGHT:
            a_Color.rgb += CalcPointLight( u_LightData[i] );
            break;
        case DIRECTION_LIGHT:
            break;
        };
    }
#endif
//    a_Color.rgb = texture( u_DiffuseMap, v_TexCoords ).rgb;
}

// -- Sharpening --
uniform float u_SharpenAmount;
uniform vec2 u_ScreenSize;

#define sharp_clamp 0.000  //[0.000 to 1.000] Limits maximum amount of sharpening a pixel recieves - Default is 0.035

// -- Advanced sharpening settings --

#define offset_bias 6.0  //[0.0 to 6.0] Offset bias adjusts the radius of the sampling pattern.
                         //I designed the pattern for offset_bias 1.0, but feel free to experiment.

#define CoefLuma vec3( 0.2126, 0.7152, 0.0722 )      // BT.709 & sRBG luma coefficient (Monitors and HD Television)
//#define CoefLuma vec3( 0.299, 0.587, 0.114 )       // BT.601 luma coefficient (SD Television)
//#define CoefLuma vec3( 1.0/3.0, 1.0/3.0, 1.0/3.0 ) // Equal weight coefficient

vec4 sharpenImage( sampler2D tex, vec2 pos )
{
	vec4 colorInput = texture2D(tex, pos);
  	
	vec3 ori = colorInput.rgb;

	// -- Combining the strength and luma multipliers --
	vec3 sharp_strength_luma = (CoefLuma * u_SharpenAmount); //I'll be combining even more multipliers with it later on
	
	// -- Gaussian filter --
	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]
	//   [ .50,   1, .50]  =  [ 2 , 4 , 2 ]
 	//   [ .25, .50, .25]     [ 1 , 2 , 1 ]


    float px = 1.0/u_ScreenSize[0];
	float py = 1.0/u_ScreenSize[1];

	vec3 blur_ori = texture2D(tex, pos + vec2(px,-py) * 0.5 * offset_bias).rgb; // South East
	blur_ori += texture2D(tex, pos + vec2(-px,-py) * 0.5 * offset_bias).rgb;  // South West
	blur_ori += texture2D(tex, pos + vec2(px,py) * 0.5 * offset_bias).rgb; // North East
	blur_ori += texture2D(tex, pos + vec2(-px,py) * 0.5 * offset_bias).rgb; // North West

	blur_ori *= 0.25;  // ( /= 4) Divide by the number of texture fetches



	// -- Calculate the sharpening --
	vec3 sharp = ori - blur_ori;  //Subtracting the blurred image from the original image

	// -- Adjust strength of the sharpening and clamp it--
	vec4 sharp_strength_luma_clamp = vec4(sharp_strength_luma * (0.5 / sharp_clamp),0.5); //Roll part of the clamp into the dot

	float sharp_luma = clamp((dot(vec4(sharp,1.0), sharp_strength_luma_clamp)), 0.0,1.0 ); //Calculate the luma, adjust the strength, scale up and clamp
	sharp_luma = (sharp_clamp * 2.0) * sharp_luma - sharp_clamp; //scale down


	// -- Combining the values to get the final sharpened pixel	--

	colorInput.rgb = colorInput.rgb + sharp_luma;    // Add the sharpening to the input color.
	return clamp(colorInput, 0.0,1.0);
}

void main() {
    a_Color = sharpenImage( u_DiffuseMap, v_TexCoords );
    if ( a_Color.a == 0.0 ) {
        discard;
    }

    ApplyLighting();

#if defined(USE_HDR)
#if !defined(USE_EXPOSURE_TONE_MAPPING)
	// reinhard tone mapping
	a_Color.rgb = a_Color.rgb / ( a_Color.rgb + vec3( 1.0 ) );
#else
	// exposure tone mapping //  BROKEN
//	a_Color.rgb = vec3( 1.0 ) - exp( -a_Color.rgb * u_Exposure );
#endif

#if defined(USE_BLOOM)
	// check whether fragment output is higher than threshold, if so output as brightness color
	float brightness = dot( a_Color.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
	if ( brightness > 1.0 ) {
		a_BrightColor = vec4( a_Color.rgb, 1.0 );
	} else {
		a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
#endif
#endif
//	a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
    a_Color.rgb *= v_Color.rgb;
}
