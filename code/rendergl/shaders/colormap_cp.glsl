layout( local_size_x = 128, local_size_y = 4, local_size_z = 1 ) in;
layout( rgba32f, binding = 0 ) uniform image2D a_ColorMap;

uniform sampler2D u_BrightMap;
uniform sampler2D u_DiffuseMap;

uniform vec3 u_AmbientColor;
uniform uvec2 u_DispatchComputeSize;

uniform int u_AntiAliasing;
uniform int u_AntiAliasingQuality;
uniform float u_GammaAmount;
uniform float u_CameraExposure;

#include "image_sharpen.glsl"
#include "fxaa.glsl"

#define DXC_STATIC_DISPATCH_GRID_DIM 0

ivec2 ThreadGroupTiling()
{
	const uvec2 dispatchGridDim = u_DispatchComputeSize;
	const uvec2 ctaDim = uvec2( 64, 16 );
	const uint maxTileWidth = 32;
	const uvec2 groupThreadID = gl_WorkGroupID.xy;
	const uvec2 groupId = gl_LocalInvocationID.xy;

	// A perfect tile is one with dimensions = [maxTileWidth, dispatchGridDim.y]
	const uint Number_of_CTAs_in_a_perfect_tile = maxTileWidth * dispatchGridDim.y;

	// Possible number of perfect tiles
	const uint Number_of_perfect_tiles = dispatchGridDim.x / maxTileWidth;

	// Total number of CTAs present in the perfect tiles
	const uint Total_CTAs_in_all_perfect_tiles = Number_of_perfect_tiles * maxTileWidth * dispatchGridDim.y;
	const uint vThreadGroupIDFlattened = dispatchGridDim.x * groupId.y + groupId.x;

	// Tile_ID_of_current_CTA : current CTA to TILE-ID mapping.
	const uint Tile_ID_of_current_CTA = vThreadGroupIDFlattened / Number_of_CTAs_in_a_perfect_tile;
	const uint Local_CTA_ID_within_current_tile = vThreadGroupIDFlattened % Number_of_CTAs_in_a_perfect_tile;
	uint Local_CTA_ID_y_within_current_tile;
	uint Local_CTA_ID_x_within_current_tile;

	if (Total_CTAs_in_all_perfect_tiles <= vThreadGroupIDFlattened)
	{
		// Path taken only if the last tile has imperfect dimensions and CTAs from the last tile are launched. 
		uint X_dimension_of_last_tile = dispatchGridDim.x % maxTileWidth;
	#ifdef DXC_STATIC_DISPATCH_GRID_DIM
		X_dimension_of_last_tile = max(1, X_dimension_of_last_tile);
	#endif
		Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / X_dimension_of_last_tile;
		Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % X_dimension_of_last_tile;
	}
	else
	{
		Local_CTA_ID_y_within_current_tile = Local_CTA_ID_within_current_tile / maxTileWidth;
		Local_CTA_ID_x_within_current_tile = Local_CTA_ID_within_current_tile % maxTileWidth;
	}

	const uint Swizzled_vThreadGroupIDFlattened =
		Tile_ID_of_current_CTA * maxTileWidth +
		Local_CTA_ID_y_within_current_tile * dispatchGridDim.x +
		Local_CTA_ID_x_within_current_tile;

	uvec2 SwizzledvThreadGroupID;
	SwizzledvThreadGroupID.y = Swizzled_vThreadGroupIDFlattened / dispatchGridDim.x;
	SwizzledvThreadGroupID.x = Swizzled_vThreadGroupIDFlattened % dispatchGridDim.x;

	uvec2 SwizzledvThreadID;
	SwizzledvThreadID.x = ctaDim.x * SwizzledvThreadGroupID.x + groupThreadID.x;
	SwizzledvThreadID.y = ctaDim.y * SwizzledvThreadGroupID.y + groupThreadID.y;

	return ivec2( SwizzledvThreadID.xy );
}

#define NUM_GAUSSIAN_PASSES 10

void CalcBloom( out vec4 outColor, ivec2 texCoords ) {
#if defined(USE_BLOOM) && defined(USE_HDR)
	vec4 brightColor = texture2D( u_BrightMap, vec2( texCoords ) );
	
	// make the threshold simply the ambient lighting so that in darker environments brighter lights
	// burn those eyes

	// don't bother with the blur if there's nothing to blur
	if ( brightColor != vec4( 0.0, 0.0, 0.0, 1.0 ) ) {
		float weight[5] = float[]( 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 );
		const vec2 texOffset = 1.0 / textureSize( u_DiffuseMap, 0 );
		vec3 result = texture2D( u_DiffuseMap, texCoords ).rgb * weight[0];
		
		for ( int p = 0; p < NUM_GAUSSIAN_PASSES; ++p ) {
			for ( int i = 1; i < 5; ++i ) {
			    result += texture2D( u_DiffuseMap, texCoords + vec2( texOffset.x * i, 0.0 ) ).rgb * weight[i];
			    result += texture2D( u_DiffuseMap, texCoords - vec2( texOffset.x * i, 0.0 ) ).rgb * weight[i];
			}
			for ( int i = 1; i < 5; ++i ) {
				result += texture2D( u_DiffuseMap, texCoords + vec2( 0.0, texOffset.y * i ) ).rgb * weight[i];
				result += texture2D( u_DiffuseMap, texCoords - vec2( 0.0, texOffset.y * i ) ).rgb * weight[i];
			}
		}
		brightColor.rgb *= result;
	}

	outColor.rgb += brightColor.rgb;
#endif
}

void main() {
	ivec2 texCoords = ThreadGroupTiling();
	vec4 outColor = texture2D( u_DiffuseMap, vec2( texCoords ) );
	
	CalcBloom( outColor, texCoords );
	
#if ANTIALIAS_TYPE == AntiAlias_FXAA
	outColor = ApplyFXAA( u_DiffuseMap, vec2( texCoords.st ) );
#endif
	outColor *= sharpenImage( u_DiffuseMap, vec2( texCoords.st ) );
	
#if defined(USE_HDR)
	outColor.rgb = vec3( 1.0 ) - exp( -outColor.rgb * u_CameraExposure );
#endif
	
	// gamma correct
	outColor.rgb = pow( outColor.rgb, vec3( 1.0 / u_GammaAmount ) );
	
	memoryBarrierImage();
	imageStore( a_ColorMap, texCoords, outColor );
}