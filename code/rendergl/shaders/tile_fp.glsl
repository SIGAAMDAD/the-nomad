#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#if defined(USE_HDR) && defined(USE_BLOOM)
layout( location = 1 ) out vec4 a_BrightColor;
#endif
#endif

#if defined(USE_FXAA)
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
#endif

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec4 v_Color;
in vec3 v_WorldPos;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;

#if defined(USE_LIGHT)
uniform vec3 u_AmbientColor;
uniform float u_AmbientIntensity;
#endif

#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
uniform vec4 u_SpecularScale;
uniform vec4 u_NormalScale;
uniform uint u_NumLights;
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

#if defined(USE_FXAA)
uniform vec2 u_ScreenSize;
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

#if defined(USE_NORMALMAP)
void CalcNormal() {
    vec3 normal = texture2D( u_NormalMap, v_TexCoords ).rgb;
    normal = normalize( normal * 2.0 - 1.0 );
    a_Color.rgb *= normal * 0.5 + 0.5;
}
#endif

void applyLighting() {
#if defined(USE_NORMALMAP)
    CalcNormal();
#endif
#if defined(USE_SPECULARMAP)
    a_Color.rgb += texture2D( u_SpecularMap, v_TexCoords ).rgb;
#endif
#if defined(USE_LIGHT) && !defined(USE_FAST_LIGHT)
    if ( u_NumLights == uint( 0 ) )
#endif
	{
        a_Color.rgb += texture2D( u_DiffuseMap, v_TexCoords ).rgb;
    }

#if defined(USE_LIGHT)
    a_Color.rgb *= u_AmbientColor;
#endif
}

#if defined(USE_FXAA)
float FxaaLuma( vec3 rgb ) {
    return rgb.y * ( 0.587 / 0.299 ) + rgb.x;
}

vec3 FxaaLerp3( vec3 a, vec3 b, float amountOfA ) {
    return ( vec3( -amountOfA ) * b ) + ( ( a * vec3( amountOfA ) ) + b );
}

vec4 FxaaTexOffset( sampler2D texture, vec2 texCoords, ivec2 offset, vec2 rcpFrame ) {
	float x = texCoords.x + float( offset.x ) + rcpFrame.x;
	float y = texCoords.y + float( offset.y ) + rcpFrame.y;
	return texture2D( texture, vec2( x, y ) );
}

// pos is the output of FxaaVertexShader interpolated across screen.
// xy -> actual texture position {0.0 to 1.0}
// rcpFrame should be a uniform equal to  {1.0/frameWidth, 1.0/frameHeight}
vec3 FxaaPixelShader(vec2 pos, sampler2D texture, vec2 rcpFrame)
{
    vec3 rgbN = FxaaTexOffset(texture, pos.xy, ivec2( 0,-1), rcpFrame).xyz;
    vec3 rgbW = FxaaTexOffset(texture, pos.xy, ivec2(-1, 0), rcpFrame).xyz;
    vec3 rgbM = FxaaTexOffset(texture, pos.xy, ivec2( 0, 0), rcpFrame).xyz;
    vec3 rgbE = FxaaTexOffset(texture, pos.xy, ivec2( 1, 0), rcpFrame).xyz;
    vec3 rgbS = FxaaTexOffset(texture, pos.xy, ivec2( 0, 1), rcpFrame).xyz;
    
    float lumaN = FxaaLuma(rgbN);
    float lumaW = FxaaLuma(rgbW);
    float lumaM = FxaaLuma(rgbM);
    float lumaE = FxaaLuma(rgbE);
    float lumaS = FxaaLuma(rgbS);
    float rangeMin = min(lumaM, min(min(lumaN, lumaW), min(lumaS, lumaE)));
    float rangeMax = max(lumaM, max(max(lumaN, lumaW), max(lumaS, lumaE)));
    
    float range = rangeMax - rangeMin;
    if(range < max(FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD)) {
        return rgbM;
    }
    
    vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    
    float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE; 
    blendL = min(FXAA_SUBPIX_CAP, blendL);
    
    vec3 rgbNW = FxaaTexOffset(texture, pos.xy, ivec2(-1,-1), rcpFrame).xyz;
    vec3 rgbNE = FxaaTexOffset(texture, pos.xy, ivec2( 1,-1), rcpFrame).xyz;
    vec3 rgbSW = FxaaTexOffset(texture, pos.xy, ivec2(-1, 1), rcpFrame).xyz;
    vec3 rgbSE = FxaaTexOffset(texture, pos.xy, ivec2( 1, 1), rcpFrame).xyz;
    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= vec3(1.0/9.0);
    
    float lumaNW = FxaaLuma(rgbNW);
    float lumaNE = FxaaLuma(rgbNE);
    float lumaSW = FxaaLuma(rgbSW);
    float lumaSE = FxaaLuma(rgbSE);
    
    float edgeVert = 
        abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
        abs((0.50 * lumaW ) + (-1.0 * lumaM) + (0.50 * lumaE )) +
        abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE));
    float edgeHorz = 
        abs((0.25 * lumaNW) + (-0.5 * lumaW) + (0.25 * lumaSW)) +
        abs((0.50 * lumaN ) + (-1.0 * lumaM) + (0.50 * lumaS )) +
        abs((0.25 * lumaNE) + (-0.5 * lumaE) + (0.25 * lumaSE));
        
    bool horzSpan = edgeHorz >= edgeVert;
    float lengthSign = horzSpan ? -rcpFrame.y : -rcpFrame.x;
    
    if(!horzSpan) {
        lumaN = lumaW;
        lumaS = lumaE;
    }
    
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    lumaN = (lumaN + lumaM) * 0.5;
    lumaS = (lumaS + lumaM) * 0.5;
    
    if (gradientN < gradientS) {
        lumaN = lumaS;
        lumaN = lumaS;
        gradientN = gradientS;
        lengthSign *= -1.0;
    }
    
    vec2 posN;
    posN.x = pos.x + (horzSpan ? 0.0 : lengthSign * 0.5);
    posN.y = pos.y + (horzSpan ? lengthSign * 0.5 : 0.0);
    
    gradientN *= FXAA_SEARCH_THRESHOLD;
    
    vec2 posP = posN;
    vec2 offNP = horzSpan ? vec2(rcpFrame.x, 0.0) : vec2(0.0, rcpFrame.y); 
    float lumaEndN = lumaN;
    float lumaEndP = lumaN;
    bool doneN = false;
    bool doneP = false;
    posN += offNP * vec2(-1.0, -1.0);
    posP += offNP * vec2( 1.0,  1.0);
    
    for(int i = 0; i < FXAA_SEARCH_STEPS; i++) {
        if(!doneN) {
            lumaEndN = FxaaLuma(texture2D(texture, posN.xy).xyz);
        }
        if(!doneP) {
            lumaEndP = FxaaLuma(texture2D(texture, posP.xy).xyz);
        }
        
        doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
        doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
        
        if(doneN && doneP) {
            break;
        }
        if(!doneN) {
            posN -= offNP;
        }
        if(!doneP) {
            posP += offNP;
        }
    }
    
    float dstN = horzSpan ? pos.x - posN.x : pos.y - posN.y;
    float dstP = horzSpan ? posP.x - pos.x : posP.y - pos.y;
    bool directionN = dstN < dstP;
    lumaEndN = directionN ? lumaEndN : lumaEndP;
    
    if( ( ( lumaM - lumaN ) < 0.0 ) == ( ( lumaEndN - lumaN ) < 0.0 ) ) {
        lengthSign = 0.0;
    }
 

    float spanLength = ( dstP + dstN );
    dstN = directionN ? dstN : dstP;
    float subPixelOffset = ( 0.5 + ( dstN * ( -1.0/spanLength ) ) ) * lengthSign;
    vec3 rgbF = texture2D( texture, vec2(
        pos.x + ( horzSpan ? 0.0 : subPixelOffset ),
        pos.y + ( horzSpan ? subPixelOffset : 0.0 ) ) ).xyz;
    return FxaaLerp3( rgbL, rgbF, blendL );
}
#endif

// -- Sharpening --
uniform float u_SharpenAmount;
uniform vec2 u_ScreenSize;

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
    a_Color = sharpenImage( u_DiffuseMap, v_TexCoords );

    applyLighting();

#if defined(USE_BLOOM) && !defined(USE_FAST_LIGHT)
	const float brightness = dot( a_Color.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
	if ( brightness > 1.0 ) {
		a_BrightColor = vec4( a_Color.rgb, 1.0 );
	} else {
		a_BrightColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	}
#endif
#if defined(USE_HDR)
#if !defined(USE_EXPOSURE_TONE_MAPPING)
	// reinhard tone mapping
	a_Color.rgb = a_Color.rgb / ( a_Color.rgb + vec3( 1.0 ) );
#else
	// exposure tone mapping
	a_Color.rgb = vec3( 1.0 ) - exp( -a_Color.rgb * u_CameraExposure );
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
    a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
}
