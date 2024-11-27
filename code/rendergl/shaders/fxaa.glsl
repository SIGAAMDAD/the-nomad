// not my work, from https://github.com/libretro/glsl-shaders/blob/master/anti-aliasing/shaders/fxaa.glsl

#if 1
/*
FXAA_PRESET - Choose compile-in knob preset 0-5.
------------------------------------------------------------------------------
FXAA_EDGE_THRESHOLD - The minimum amount of local contrast required 
					  to apply algorithm.
					  1.0/3.0  - too little
					  1.0/4.0  - good start
					  1.0/8.0  - applies to more edges
					  1.0/16.0 - overkill
------------------------------------------------------------------------------
FXAA_EDGE_THRESHOLD_MIN - Trims the algorithm from processing darks.
						  Perf optimization.
						  1.0/32.0 - visible limit (smaller isn't visible)
						  1.0/16.0 - good compromise
						  1.0/12.0 - upper limit (seeing artifacts)
------------------------------------------------------------------------------
FXAA_SEARCH_STEPS - Maximum number of search steps for end of span.
------------------------------------------------------------------------------
FXAA_SEARCH_THRESHOLD - Controls when to stop searching.
						1.0/4.0 - seems to be the best quality wise
------------------------------------------------------------------------------
FXAA_SUBPIX_TRIM - Controls sub-pixel aliasing removal.
				   1.0/2.0 - low removal
				   1.0/3.0 - medium removal
				   1.0/4.0 - default removal
				   1.0/8.0 - high removal
				   0.0 - complete removal
------------------------------------------------------------------------------
FXAA_SUBPIX_CAP - Insures fine detail is not completely removed.
				  This is important for the transition of sub-pixel detail,
				  like fences and wires.
				  3.0/4.0 - default (medium amount of filtering)
				  7.0/8.0 - high amount of filtering
				  1.0 - no capping of sub-pixel aliasing removal
*/

// Return the luma, the estimation of luminance from rgb inputs.
// This approximates luma using one FMA instruction,
// skipping normalization and tossing out blue.
// FxaaLuma() will range 0.0 to 2.963210702.
float FxaaLuma( vec3 rgb ) {
	return rgb.y * ( 0.587 / 0.299 ) + rgb.x;
}

vec3 FxaaLerp3( vec3 a, vec3 b, float amountOfA ) {
	return ( vec3( -amountOfA ) * b) + ( ( a * vec3( amountOfA ) ) + b );
}

vec4 FxaaTexOff( sampler2D tex, vec2 pos, ivec2 off, vec2 rcpFrame ) {
	float x = pos.x + float( off.x ) * rcpFrame.x;
	float y = pos.y + float( off.y ) * rcpFrame.y;
	return texture( tex, vec2( x, y ) );
}

// pos is the output of FxaaVertexShader interpolated across screen.
// xy -> actual texture position {0.0 to 1.0}
// rcpFrame should be a uniform equal to  {1.0/frameWidth, 1.0/frameHeight}
vec4 ApplyFXAA( sampler2D tex, vec2 pos ) {
	vec2 rcpFrame = vec2( 1.0 / u_ScreenSize.x, 1.0 / u_ScreenSize.y );

	float FXAA_EDGE_THRESHOLD = 0.0;
	float FXAA_EDGE_THRESHOLD_MIN = 0.0;
	int FXAA_SEARCH_STEPS = 0;
	float FXAA_SEARCH_THRESHOLD = 0.0;
	float FXAA_SUBPIX_CAP = 0.0;
	float FXAA_SUBPIX_TRIM = 0.0;

#if !defined(USE_SWITCH)
	if ( u_AntiAliasingQuality == QUALITY_LOW ) {
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 16.0;
		FXAA_SEARCH_STEPS		= 16;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
	}
	else if ( u_AntiAliasingQuality == QUALITY_NORMAL ) {
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 24.0;
		FXAA_SEARCH_STEPS		= 24;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
	}
	else if ( u_AntiAliasingQuality == QUALITY_HIGH ) {
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 24.0;
		FXAA_SEARCH_STEPS		= 32;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
	}
#else
	switch ( u_AntiAliasingQuality ) {
	case QUALITY_LOW:
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 16.0;
		FXAA_SEARCH_STEPS		= 16;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
		break;
	case QUALITY_NORMAL:
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 24.0;
		FXAA_SEARCH_STEPS		= 24;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
		break;
	case QUALITY_HIGH:
		FXAA_EDGE_THRESHOLD		= 1.0 / 8.0;
		FXAA_EDGE_THRESHOLD_MIN	= 1.0 / 24.0;
		FXAA_SEARCH_STEPS		= 32;
		FXAA_SEARCH_THRESHOLD	= ( 1.0 / 4.0 );
		FXAA_SUBPIX_CAP			= ( 3.0 / 4.0 );
		FXAA_SUBPIX_TRIM		= ( 1.0 / 4.0 );
		break;
	};
#endif

	float FXAA_SUBPIX_TRIM_SCALE = ( 1.0 / ( 1.0 - FXAA_SUBPIX_TRIM ) );

	vec3 rgbN = FxaaTexOff( tex, pos.xy, ivec2(  0,-1 ), rcpFrame ).xyz;
	vec3 rgbW = FxaaTexOff( tex, pos.xy, ivec2( -1, 0 ), rcpFrame ).xyz;
	vec3 rgbM = FxaaTexOff( tex, pos.xy, ivec2(  0, 0 ), rcpFrame ).xyz;
	vec3 rgbE = FxaaTexOff( tex, pos.xy, ivec2(  1, 0 ), rcpFrame ).xyz;
	vec3 rgbS = FxaaTexOff( tex, pos.xy, ivec2(  0, 1 ), rcpFrame ).xyz;
	
	float lumaN = FxaaLuma( rgbN );
	float lumaW = FxaaLuma( rgbW );
	float lumaM = FxaaLuma( rgbM );
	float lumaE = FxaaLuma( rgbE );
	float lumaS = FxaaLuma( rgbS );
	float rangeMin = min( lumaM, min( min( lumaN, lumaW ), min( lumaS, lumaE ) ) );
	float rangeMax = max( lumaM, max( max( lumaN, lumaW ), max( lumaS, lumaE ) ) );
	
	float range = rangeMax - rangeMin;
	if ( range < max( FXAA_EDGE_THRESHOLD_MIN, rangeMax * FXAA_EDGE_THRESHOLD ) ) {
		return vec4( rgbM, 1.0 );
	}
	
	vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
	
	float lumaL = ( lumaN + lumaW + lumaE + lumaS ) * 0.25;
	float rangeL = abs(lumaL - lumaM);
	float blendL = max(0.0, (rangeL / range) - FXAA_SUBPIX_TRIM) * FXAA_SUBPIX_TRIM_SCALE; 
	blendL = min(FXAA_SUBPIX_CAP, blendL);
	
	vec3 rgbNW = FxaaTexOff(tex, pos.xy, ivec2(-1,-1), rcpFrame).xyz;
	vec3 rgbNE = FxaaTexOff(tex, pos.xy, ivec2( 1,-1), rcpFrame).xyz;
	vec3 rgbSW = FxaaTexOff(tex, pos.xy, ivec2(-1, 1), rcpFrame).xyz;
	vec3 rgbSE = FxaaTexOff(tex, pos.xy, ivec2( 1, 1), rcpFrame).xyz;
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
	
	if(!horzSpan)
	{
		lumaN = lumaW;
		lumaS = lumaE;
	}
	
	float gradientN = abs(lumaN - lumaM);
	float gradientS = abs(lumaS - lumaM);
	lumaN = (lumaN + lumaM) * 0.5;
	lumaS = (lumaS + lumaM) * 0.5;
	
	if (gradientN < gradientS)
	{
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
		if(!doneN)
		{
			lumaEndN = FxaaLuma(texture(tex, posN.xy).xyz);
		}
		if(!doneP)
		{
			lumaEndP = FxaaLuma(texture(tex, posP.xy).xyz);
		}
		
		doneN = doneN || (abs(lumaEndN - lumaN) >= gradientN);
		doneP = doneP || (abs(lumaEndP - lumaN) >= gradientN);
		
		if(doneN && doneP)
		{
			break;
		}
		if(!doneN)
		{
			posN -= offNP;
		}
		if(!doneP)
		{
			posP += offNP;
		}
	}
	
	float dstN = horzSpan ? pos.x - posN.x : pos.y - posN.y;
	float dstP = horzSpan ? posP.x - pos.x : posP.y - pos.y;
	bool directionN = dstN < dstP;
	lumaEndN = directionN ? lumaEndN : lumaEndP;
	
	if(((lumaM - lumaN) < 0.0) == ((lumaEndN - lumaN) < 0.0))
	{
		lengthSign = 0.0;
	}
 

	float spanLength = (dstP + dstN);
	dstN = directionN ? dstN : dstP;
	float subPixelOffset = (0.5 + (dstN * (-1.0/spanLength))) * lengthSign;
	vec3 rgbF = texture(tex, vec2(
		pos.x + (horzSpan ? 0.0 : subPixelOffset),
		pos.y + (horzSpan ? subPixelOffset : 0.0))).xyz;
	
	return vec4( FxaaLerp3( rgbL, rgbF, blendL ), 1.0 ); 
}
#else
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
	vec3 rgbNW = texture(tex, v_rgbNW).xyz;
	vec3 rgbNE = texture(tex, v_rgbNE).xyz;
	vec3 rgbSW = texture(tex, v_rgbSW).xyz;
	vec3 rgbSE = texture(tex, v_rgbSE).xyz;
	vec4 texColor = texture(tex, v_rgbM);
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
		texture(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
		texture(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (
		texture(tex, fragCoord * inverseVP + dir * -0.5).xyz +
		texture(tex, fragCoord * inverseVP + dir * 0.5).xyz);

	float lumaB = dot(rgbB, luma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
		color = vec4(rgbA, texColor.a);
	else
		color = vec4(rgbB, texColor.a);
	return color;
}

vec4 ApplyFXAA( sampler2D tex, vec2 fragCoord ) {
	vec2 resolution = u_ScreenSize;
	vec2 v_rgbNW;
	vec2 v_rgbNE;
	vec2 v_rgbSW;
	vec2 v_rgbSE;
	vec2 v_rgbM;

	texcoords( fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );

	return fxaa( tex, fragCoord, resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM );
}
#endif