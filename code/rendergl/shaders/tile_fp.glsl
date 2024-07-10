#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#if defined(USE_HDR) && defined(USE_BLOOM)
layout( location = 1 ) out vec4 a_BrightColor;
#endif
#endif

#if !defined(FXAA_PRESET)
    #define FXAA_PRESET 5
#endif
#if FXAA_PRESET == 3
    #define FXAA_EDGE_THRESHOLD      ( 1.0 / 8.0 )
    #define FXAA_EDGE_THRESHOLD_MIN  ( 1.0 / 16.0 )
    #define FXAA_SEARCH_STEPS        16
    #define FXAA_SEARCH_THRESHOLD    ( 1.0 / 4.0 )
    #define FXAA_SUBPIX_CAP          ( 3.0 / 4.0 )
    #define FXAA_SUBPIX_TRIM         ( 1.0 / 4.0 )
#elif FXAA_PRESET == 4
    #define FXAA_EDGE_THRESHOLD      ( 1.0 / 8.0 )
    #define FXAA_EDGE_THRESHOLD_MIN  ( 1.0 / 24.0 )
    #define FXAA_SEARCH_STEPS        24
    #define FXAA_SEARCH_THRESHOLD    ( 1.0 / 4.0 )
    #define FXAA_SUBPIX_CAP          ( 3.0 / 4.0 )
    #define FXAA_SUBPIX_TRIM         ( 1.0 / 4.0 )
#elif FXAA_PRESET == 5
    #define FXAA_EDGE_THRESHOLD      ( 1.0 / 8.0 )
    #define FXAA_EDGE_THRESHOLD_MIN  ( 1.0 / 24.0 )
    #define FXAA_SEARCH_STEPS        32
    #define FXAA_SEARCH_THRESHOLD    ( 1.0 / 4.0 )
    #define FXAA_SUBPIX_CAP          ( 3.0 / 4.0 )
    #define FXAA_SUBPIX_TRIM         ( 1.0 / 4.0 )
#endif

#define FXAA_SUBPIX_TRIM_SCALE ( 1.0 / ( 1.0 - FXAA_SUBPIX_TRIM ) )

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec4 v_Color;
in vec3 v_WorldPos;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;
uniform bool u_GamePaused;
uniform bool u_HardwareGamma;
uniform int u_AntiAliasing;

#if defined(USE_LIGHT)
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;
uniform int u_NumLights;

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

#if defined(USE_EXPOSURE_TONE_MAPPING)
uniform float u_CameraExposure;
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

uniform vec2 u_ScreenSize;
uniform float u_SharpenAmount;
uniform int u_AlphaTest;

void texcoords( vec2 fragCoord, vec2 resolution, out vec2 v_rgbNW, out vec2 v_rgbNE, out vec2 v_rgbSW,
	out vec2 v_rgbSE, out vec2 v_rgbM )
{
	vec2 inverseVP = 1.0 / resolution.xy;
	v_rgbNW = ( fragCoord + vec2( -1.0, -1.0 ) ) * inverseVP;
	v_rgbNE = ( fragCoord + vec2( 1.0, -1.0 ) ) * inverseVP;
	v_rgbSW = ( fragCoord + vec2( -1.0, 1.0 ) ) * inverseVP;
	v_rgbSE = ( fragCoord + vec2( 1.0, 1.0 ) ) * inverseVP;
	v_rgbM = vec2( fragCoord * inverseVP );
}

#ifndef FXAA_REDUCE_MIN
    #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
    #define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#ifndef FXAA_SPAN_MAX
    #define FXAA_SPAN_MAX     8.0
#endif

//optimized version for mobile, where dependent 
//texture reads can be a bottleneck
vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
            vec2 v_rgbNW, vec2 v_rgbNE, 
            vec2 v_rgbSW, vec2 v_rgbSE, 
            vec2 v_rgbM) {
    vec4 color;
    vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
    vec3 rgbNW = texture2D(tex, v_rgbNW).xyz;
    vec3 rgbNE = texture2D(tex, v_rgbNE).xyz;
    vec3 rgbSW = texture2D(tex, v_rgbSW).xyz;
    vec3 rgbSE = texture2D(tex, v_rgbSE).xyz;
    vec4 texColor = texture2D(tex, v_rgbM);
    vec3 rgbM  = texColor.xyz;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
              dir * rcpDirMin)) * inverseVP;
    
    vec3 rgbA = 0.5 * (
        texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2D(tex, fragCoord * inverseVP + dir * -0.5).xyz +
        texture2D(tex, fragCoord * inverseVP + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}

vec4 applyFXAA( sampler2D tex, vec2 fragCoord, vec2 resolution ) {
	vec2 v_rgbNW;
	vec2 v_rgbNE;
	vec2 v_rgbSW;
	vec2 v_rgbSE;
	vec2 v_rgbM;

	texcoords( fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );

	return fxaa( tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );
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

void CalcNormal() {
#if defined(USE_NORMALMAP)
    vec3 normal = texture( u_NormalMap, v_TexCoords ).rgb;
    normal = normalize( normal * 2.0 - 1.0 );
    a_Color.rgb *= normal * 0.5 + 0.5;
#endif
}

void ApplyLighting() {
    a_Color = texture( u_DiffuseMap, v_TexCoords );
    CalcNormal();
#if defined(USE_SPECULARMAP)
    if ( u_NumLights == 0 ) {
        a_Color.rgb += texture( u_SpecularMap, v_TexCoords ).rgb;
    }
#endif
#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
    for ( int i = 0; i < u_NumLights; i++ ) {
        switch ( lights[i].type ) {
        case POINT_LIGHT:
            a_Color.rgb += CalcPointLight( u_LightData[i] );
            break;
        case DIRECTION_LIGHT:
            break;
        };
    }
#endif
    a_Color.rgb += texture( u_DiffuseMap, v_TexCoords ).rgb;
}

#define sharp_clamp 0.000  //[0.000 to 1.000] Limits maximum amount of sharpening a pixel recieves - Default is 0.035

// -- Advanced sharpening settings --

#define offset_bias 6.0  //[0.0 to 6.0] Offset bias adjusts the radius of the sampling pattern.
                         //I designed the pattern for offset_bias 1.0, but feel free to experiment.

//#define CoefLuma vec3( 0.2126, 0.7152, 0.0722 )      // BT.709 & sRBG luma coefficient (Monitors and HD Television)
//#define CoefLuma vec3( 0.299, 0.587, 0.114 )       // BT.601 luma coefficient (SD Television)
#define CoefLuma vec3( 1.0/3.0, 1.0/3.0, 1.0/3.0 ) // Equal weight coefficient

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
    if ( u_AntiAliasing == AntiAlias_FXAA ) {
        vec2 fragCoord = v_TexCoords * u_ScreenSize;
        a_Color = applyFXAA( u_DiffuseMap, fragCoord, u_ScreenSize );
    } else {
        a_Color = sharpenImage( u_DiffuseMap, v_TexCoords );
    }

    ApplyLighting();

#if defined(USE_HDR)
#if !defined(USE_EXPOSURE_TONE_MAPPING)
	// reinhard tone mapping
	a_Color.rgb = a_Color.rgb / ( a_Color.rgb + vec3( 1.0 ) );
#else
	// exposure tone mapping
	a_Color.rgb = vec3( 1.0 ) - exp( -a_Color.rgb * u_CameraExposure );
#endif
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
    if ( u_HardwareGamma ) {
        a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
    }
    if ( u_GamePaused ) {
        a_Color.rgb = vec3( 0.75, 0.75, 0.75 );
    }
}
