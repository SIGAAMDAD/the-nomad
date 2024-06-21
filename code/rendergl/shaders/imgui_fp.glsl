#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
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

#if !defined(FXAA_REDUCE_MIN)
    #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#if !defined(FXAA_REDUCE_MUL)
    #define FXAA_REDUCE_MUL   (1.0 / 8.0)
#endif
#if !defined(FXAA_SPAN_MAX)
    #define FXAA_SPAN_MAX     8.0
#endif
#endif

in vec2 v_TexCoords;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;
uniform float u_GammaAmount;

uniform vec2 u_ScreenSize;

#if defined(USE_SMAA) && defined(USE_LUMA_SMAA_EDGE)
#if !defined(SMAA_THRESHOLD)
#define SMAA_THRESHOLD 0.1
#endif
#if !defined(SMAA_MAX_SEARCH_STEPS)
#define SMAA_MAX_SEARCH_STEPS 16
#endif
#if !defined(SMAA_MAX_SEARCH_STEPS_DIAG)
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#endif
#if !defined(SMAA_CORNER_ROUNDING)
#define SMAA_CORNER_ROUNDING 25
#endif

#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_SEARCHTEX_SIZE vec2(66.0, 33.0)
#define SMAA_SEARCHTEX_PACKED_SIZE vec2(64.0, 16.0)
#define SMAA_CORNER_ROUNDING_NORM (float(SMAA_CORNER_ROUNDING) / 100.0)

#if !defined(SMAA_AREATEX_SELECT)
#define SMAA_AREATEX_SELECT(sample) sample.rg
#endif

#if !defined(SMAA_SEARCHTEX_SELECT)
#define SMAA_SEARCHTEX_SELECT(sample) sample.r
#endif

in vec2 v_PixCoord;
in vec4 v_Offset[3];

vec4 SMAA_RT_METRICS = vec4( 1.0 / u_ScreenSize.x, 1.0 / u_ScreenSize.y, u_ScreenSize.x, u_ScreenSize.y );

#define mad(a, b, c) (a * b + c)
#define saturate(a) clamp(a, 0.0, 1.0)
#define round(v) floor(v + 0.5)
#define SMAASampleLevelZeroOffset(tex, coord, offset) texture2D(tex, coord + offset * SMAA_RT_METRICS.xy)

//
// Conditional move:
//
void SMAAMovc(bvec2 cond, inout vec2 variable, vec2 value) {
  if (cond.x) variable.x = value.x;
  if (cond.y) variable.y = value.y;
}

void SMAAMovc(bvec4 cond, inout vec4 variable, vec4 value) {
  SMAAMovc(cond.xy, variable.xy, value.xy);
  SMAAMovc(cond.zw, variable.zw, value.zw);
}

//
// Allows to decode two binary values from a bilinear-filtered access.
//
vec2 SMAADecodeDiagBilinearAccess(vec2 e) {
  // Bilinear access for fetching 'e' have a 0.25 offset, and we are
  // interested in the R and G edges:
  //
  // +---G---+-------+
  // |   x o R   x   |
  // +-------+-------+
  //
  // Then, if one of these edge is enabled:
  //   Red:   (0.75 * X + 0.25 * 1) => 0.25 or 1.0
  //   Green: (0.75 * 1 + 0.25 * X) => 0.75 or 1.0
  //
  // This function will unpack the values (mad + mul + round):
  // wolframalpha.com: round(x * abs(5 * x - 5 * 0.75)) plot 0 to 1
  e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);
  return round(e);
}

vec4 SMAADecodeDiagBilinearAccess(vec4 e) {
  e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);
  return round(e);
}

//
// These functions allows to perform diagonal pattern searches.
//
vec2 SMAASearchDiag1(sampler2D edgesTex, vec2 texcoord, vec2 dir, out vec2 e) {
  vec4 coord = vec4(texcoord, -1.0, 1.0);
  vec3 t = vec3(SMAA_RT_METRICS.xy, 1.0);

  for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) {
    if (!(coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && coord.w > 0.9)) break;
    coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);
    e = texture2D(edgesTex, coord.xy).rg; // LinearSampler
    coord.w = dot(e, vec2(0.5, 0.5));
  }
  return coord.zw;
}

vec2 SMAASearchDiag2(sampler2D edgesTex, vec2 texcoord, vec2 dir, out vec2 e) {
  vec4 coord = vec4(texcoord, -1.0, 1.0);
  coord.x += 0.25 * SMAA_RT_METRICS.x; // See @SearchDiag2Optimization
  vec3 t = vec3(SMAA_RT_METRICS.xy, 1.0);

  for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) {
    if (!(coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && coord.w > 0.9)) break;
    coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);

    // @SearchDiag2Optimization
    // Fetch both edges at once using bilinear filtering:
    e = texture2D(edgesTex, coord.xy).rg; // LinearSampler
    e = SMAADecodeDiagBilinearAccess(e);

    // Non-optimized version:
    // e.g = texture2D(edgesTex, coord.xy).g; // LinearSampler
    // e.r = SMAASampleLevelZeroOffset(edgesTex, coord.xy, vec2(1, 0)).r;

    coord.w = dot(e, vec2(0.5, 0.5));
  }
  return coord.zw;
}

//
// Similar to SMAAArea, this calculates the area corresponding to a certain
// diagonal distance and crossing edges 'e'.
//
vec2 SMAAAreaDiag(sampler2D areaTex, vec2 dist, vec2 e, float offset) {
  vec2 texcoord = mad(vec2(SMAA_AREATEX_MAX_DISTANCE_DIAG, SMAA_AREATEX_MAX_DISTANCE_DIAG), e, dist);

  // We do a scale and bias for mapping to texel space:
  texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);

  // Diagonal areas are on the second half of the texture:
  texcoord.x += 0.5;

  // Move to proper place, according to the subpixel offset:
  texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;

  // Do it!
  return SMAA_AREATEX_SELECT(texture2D(areaTex, texcoord)); // LinearSampler
}

//
// This searches for diagonal patterns and returns the corresponding weights.
//
vec2 SMAACalculateDiagWeights(sampler2D edgesTex, sampler2D areaTex, vec2 texcoord, vec2 e, vec4 subsampleIndices) {
  vec2 weights = vec2(0.0, 0.0);

  // Search for the line ends:
  vec4 d;
  vec2 end;
  if (e.r > 0.0) {
      d.xz = SMAASearchDiag1(edgesTex, texcoord, vec2(-1.0,  1.0), end);
      d.x += float(end.y > 0.9);
  } else
      d.xz = vec2(0.0, 0.0);
  d.yw = SMAASearchDiag1(edgesTex, texcoord, vec2(1.0, -1.0), end);

  if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
    // Fetch the crossing edges:
    vec4 coords = mad(vec4(-d.x + 0.25, d.x, d.y, -d.y - 0.25), SMAA_RT_METRICS.xyxy, texcoord.xyxy);
    vec4 c;
    c.xy = SMAASampleLevelZeroOffset(edgesTex, coords.xy, vec2(-1,  0)).rg;
    c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, vec2( 1,  0)).rg;
    c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);

    // Non-optimized version:
    // vec4 coords = mad(vec4(-d.x, d.x, d.y, -d.y), SMAA_RT_METRICS.xyxy, texcoord.xyxy);
    // vec4 c;
    // c.x = SMAASampleLevelZeroOffset(edgesTex, coords.xy, vec2(-1,  0)).g;
    // c.y = SMAASampleLevelZeroOffset(edgesTex, coords.xy, vec2( 0,  0)).r;
    // c.z = SMAASampleLevelZeroOffset(edgesTex, coords.zw, vec2( 1,  0)).g;
    // c.w = SMAASampleLevelZeroOffset(edgesTex, coords.zw, vec2( 1, -1)).r;

    // Merge crossing edges at each side into a single value:
    vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

    // Remove the crossing edge if we didn't found the end of the line:
    SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0, 0.0));

    // Fetch the areas for this line:
    weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.z);
  }

  // Search for the line ends:
  d.xz = SMAASearchDiag2(edgesTex, texcoord, vec2(-1.0, -1.0), end);
  if (SMAASampleLevelZeroOffset(edgesTex, texcoord, vec2(1, 0)).r > 0.0) {
    d.yw = SMAASearchDiag2(edgesTex, texcoord, vec2(1.0, 1.0), end);
    d.y += float(end.y > 0.9);
  } else {
    d.yw = vec2(0.0, 0.0);
  }

  if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
    // Fetch the crossing edges:
    vec4 coords = mad(vec4(-d.x, -d.x, d.y, d.y), SMAA_RT_METRICS.xyxy, texcoord.xyxy);
    vec4 c;
    c.x  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, vec2(-1,  0)).g;
    c.y  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, vec2( 0, -1)).r;
    c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, vec2( 1,  0)).gr;
    vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

    // Remove the crossing edge if we didn't found the end of the line:
    SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0, 0.0));

    // Fetch the areas for this line:
    weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.w).gr;
  }

  return weights;
}

//
// This allows to determine how much length should we add in the last step
// of the searches. It takes the bilinearly interpolated edge (see
// @PSEUDO_GATHER4), and adds 0, 1 or 2, depending on which edges and
// crossing edges are active.
//
float SMAASearchLength(sampler2D searchTex, vec2 e, float offset) {
  // The texture is flipped vertically, with left and right cases taking half
  // of the space horizontally:
  vec2 scale = SMAA_SEARCHTEX_SIZE * vec2(0.5, -1.0);
  vec2 bias = SMAA_SEARCHTEX_SIZE * vec2(offset, 1.0);

  // Scale and bias to access texel centers:
  scale += vec2(-1.0,  1.0);
  bias  += vec2( 0.5, -0.5);

  // Convert from pixel coordinates to texcoords:
  // (We use SMAA_SEARCHTEX_PACKED_SIZE because the texture is cropped)
  scale *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
  bias *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;

  // Lookup the search texture:
  return SMAA_SEARCHTEX_SELECT(texture2D(searchTex, mad(scale, e, bias))); // LinearSampler
}

//
// Horizontal/vertical search functions for the 2nd pass.
//
float SMAASearchXLeft(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {
  //
  // @PSEUDO_GATHER4
  // This texcoord has been offset by (-0.25, -0.125) in the vertex shader to
  // sample between edge, thus fetching four edges in a row.
  // Sampling with different offsets in each direction allows to disambiguate
  // which edges are active from the four fetched ones.
  //
  vec2 e = vec2(0.0, 1.0);
  for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) {
    if (!(texcoord.x > end && e.g > 0.8281 && e.r == 0.0)) break;
    e = texture2D(edgesTex, texcoord).rg; // LinearSampler
    texcoord = mad(-vec2(2.0, 0.0), SMAA_RT_METRICS.xy, texcoord);
  }

  float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.0), 3.25);
  return mad(SMAA_RT_METRICS.x, offset, texcoord.x);

  // Non-optimized version:
  // We correct the previous (-0.25, -0.125) offset we applied:
  // texcoord.x += 0.25 * SMAA_RT_METRICS.x;

  // The searches are bias by 1, so adjust the coords accordingly:
  // texcoord.x += SMAA_RT_METRICS.x;

  // Disambiguate the length added by the last step:
  // texcoord.x += 2.0 * SMAA_RT_METRICS.x; // Undo last step
  // texcoord.x -= SMAA_RT_METRICS.x * (255.0 / 127.0) * SMAASearchLength(searchTex, e, 0.0);
  // return mad(SMAA_RT_METRICS.x, offset, texcoord.x);
}

float SMAASearchXRight(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {
  vec2 e = vec2(0.0, 1.0);
  for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) { if (!(texcoord.x < end && e.g > 0.8281 && e.r == 0.0)) break;
    e = texture2D(edgesTex, texcoord).rg; // LinearSampler
    texcoord = mad(vec2(2.0, 0.0), SMAA_RT_METRICS.xy, texcoord);
  }
  float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.5), 3.25);
  return mad(-SMAA_RT_METRICS.x, offset, texcoord.x);
}

float SMAASearchYUp(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {
  vec2 e = vec2(1.0, 0.0);
  for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) { if (!(texcoord.y > end && e.r > 0.8281 && e.g == 0.0)) break;
    e = texture2D(edgesTex, texcoord).rg; // LinearSampler
    texcoord = mad(-vec2(0.0, 2.0), SMAA_RT_METRICS.xy, texcoord);
  }
  float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.0), 3.25);
  return mad(SMAA_RT_METRICS.y, offset, texcoord.y);
}

float SMAASearchYDown( sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end ) {
    vec2 e = vec2(1.0, 0.0);
    for (int i = 0; i < SMAA_MAX_SEARCH_STEPS; i++) { if (!(texcoord.y < end && e.r > 0.8281 && e.g == 0.0)) break;
      e = texture2D(edgesTex, texcoord).rg; // LinearSampler
      texcoord = mad(vec2(0.0, 2.0), SMAA_RT_METRICS.xy, texcoord);
    }
    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.5), 3.25);
    return mad(-SMAA_RT_METRICS.y, offset, texcoord.y);
}

//
// Ok, we have the distance and both crossing edges. So, what are the areas
// at each side of current edge?
//
vec2 SMAAArea( sampler2D areaTex, vec2 dist, float e1, float e2, float offset ) {
    // Rounding prevents precision errors of bilinear filtering:
    vec2 texcoord = mad( vec2( SMAA_AREATEX_MAX_DISTANCE, SMAA_AREATEX_MAX_DISTANCE ), round( 4.0 * vec2( e1, e2 ) ), dist );

    // We do a scale and bias for mapping to texel space:
    texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);

    // Move to proper place, according to the subpixel offset:
    texcoord.y = mad(SMAA_AREATEX_SUBTEX_SIZE, offset, texcoord.y);

    // Do it!
    return SMAA_AREATEX_SELECT( texture2D( areaTex, texcoord ) ); // LinearSampler
}

// Corner Detection Functions
void SMAADetectHorizontalCornerPattern( sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d ) {
#if !defined(SMAA_DISABLE_CORNER_DETECTION)
    vec2 leftRight = step( d.xy, d.yx );
    vec2 rounding = ( 1.0 - SMAA_CORNER_ROUNDING_NORM ) * leftRight;

    rounding /= leftRight.x + leftRight.y; // Reduce blending for pixels in the center of a line.

    vec2 factor = vec2( 1.0, 1.0 );
    factor.x -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, vec2( 0,  1 ) ).r;
    factor.x -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, vec2( 1,  1 ) ).r;
    factor.y -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, vec2( 0, -2 ) ).r;
    factor.y -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, vec2( 1, -2 ) ).r;

    weights *= saturate( factor );
#endif
}

void SMAADetectVerticalCornerPattern( sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d ) {
#if !defined(SMAA_DISABLE_CORNER_DETECTION)
    vec2 leftRight = step( d.xy, d.yx );
    vec2 rounding = ( 1.0 - SMAA_CORNER_ROUNDING_NORM ) * leftRight;

    rounding /= leftRight.x + leftRight.y;

    vec2 factor = vec2( 1.0, 1.0 );
    factor.x -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, vec2(  1, 0 ) ).g;
    factor.x -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, vec2(  1, 1 ) ).g;
    factor.y -= rounding.x * SMAASampleLevelZeroOffset( edgesTex, texcoord.xy, vec2( -2, 0 ) ).g;
    factor.y -= rounding.y * SMAASampleLevelZeroOffset( edgesTex, texcoord.zw, vec2( -2, 1 ) ).g;

    weights *= saturate(factor);
#endif
}
#endif

#if defined(USE_FXAA)
vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
            vec2 v_rgbNW, vec2 v_rgbNE, 
            vec2 v_rgbSW, vec2 v_rgbSE, 
            vec2 v_rgbM) {
    vec4 color;
    mediump vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
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
    
    mediump vec2 dir;
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

void texcoords(vec2 fragCoord, vec2 resolution,
			out vec2 v_rgbNW, out vec2 v_rgbNE,
			out vec2 v_rgbSW, out vec2 v_rgbSE,
			out vec2 v_rgbM) {
	vec2 inverseVP = 1.0 / resolution.xy;
	v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
	v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
	v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
	v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
	v_rgbM = vec2(fragCoord * inverseVP);
}
#endif

#if 0

#define SHARPEN_FACTOR 4.0

vec4 sharpenMask (sampler2D tex, vec2 fragCoord)
{
    // Sharpen detection matrix [0,1,0],[1,-4,1],[0,1,0]
    // Colors
    vec4 up = texture (tex, (fragCoord + vec2 (0, 1))/u_ScreenSize.xy);
    vec4 left = texture (tex, (fragCoord + vec2 (-1, 0))/u_ScreenSize.xy);
    vec4 center = texture (tex, fragCoord/u_ScreenSize.xy);
    vec4 right = texture (tex, (fragCoord + vec2 (1, 0))/u_ScreenSize.xy);
    vec4 down = texture (tex, (fragCoord + vec2 (0, -1))/u_ScreenSize.xy);
    
    // Return edge detection
    return (1.0 + 4.0*SHARPEN_FACTOR)*center -SHARPEN_FACTOR*(up + left + right + down);
}

vec4 sharpen( sampler2D tex, in vec2 coords ) {
    float dx = 1.0 / u_ScreenSize.x;
    float dy = 1.0 / u_ScreenSize.y;
    vec4 sum = vec4(0.0);
    sum += -1. * texture2D(tex, coords + vec2( -1.0 * dx , 0.0 * dy));
    sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , -1.0 * dy));
    sum += 5. * texture2D(tex, coords + vec2( 0.0 * dx , 0.0 * dy));
    sum += -1. * texture2D(tex, coords + vec2( 0.0 * dx , 1.0 * dy));
    sum += -1. * texture2D(tex, coords + vec2( 1.0 * dx , 0.0 * dy));
    return sum;
}
#endif

// -- Sharpening --
uniform float u_SharpenAmount;

#define sharp_clamp 20.000  //[0.000 to 1.000] Limits maximum amount of sharpening a pixel recieves - Default is 0.035

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
#if defined(USE_FXAA)
	vec2 rgbNW, rgbNE, rgbSW, rgbSE, rgbM;
	
	texcoords( v_TexCoords, u_ScreenSize, rgbNW,rgbNE, rgbSW, rgbSE, rgbM );
	a_Color = v_Color * fxaa( u_DiffuseMap, v_TexCoords, u_ScreenSize, rgbNW, rgbNE, rgbSW, rgbSE, rgbM );
#endif
#if defined(USE_SMAA) && defined(USE_LUMA_SMAA_EDGE)
    vec2 threshold = vec2( SMAA_THRESHOLD );

    // Calculate lumas:
    vec4 weights = vec4( 0.2126, 0.7152, 0.0722, 0.0 );
    float L = dot( texture2D( u_DiffuseMap, v_TexCoords ).rgb, vec3( weights ) );

    float Lleft = dot( texture2D( u_DiffuseMap, v_Offset[0].xy ).rgb, vec3( weights ) );
    float Ltop  = dot( texture2D( u_DiffuseMap, v_Offset[0].zw ).rgb, vec3( weights ) );

    // We do the usual threshold:
    vec4 delta;
    delta.xy = abs( L - vec2( Lleft, Ltop ) );
    vec2 edges = step( threshold, delta.xy );

    // Then discard if there is no edge:
    if ( dot( edges, vec2( 1.0, 1.0 ) ) == 0.0 ) {
        discard;
    }

    // Calculate right and bottom deltas:
    float Lright = dot( texture2D( u_DiffuseMap, v_Offset[1].xy ).rgb, vec3( weights ) );
    float Lbottom  = dot( texture2D( u_DiffuseMap, v_Offset[1].zw ).rgb, vec3( weights ) );
    delta.zw = abs( L - vec2( Lright, Lbottom ) );

    // Calculate the maximum delta in the direct neighborhood:
    vec2 maxDelta = max( delta.xy, delta.zw );

    // Calculate left-left and top-top deltas:
    float Lleftleft = dot( texture2D( u_DiffuseMap, v_Offset[2].xy ).rgb, vec3( weights ) );
    float Ltoptop = dot( texture2D( u_DiffuseMap, v_Offset[2].zw ).rgb, vec3( weights ) );
    delta.zw = abs( vec2( Lleft, Ltop ) - vec2( Lleftleft, Ltoptop ) );

    // Calculate the final maximum delta:
    maxDelta = max(maxDelta.xy, delta.zw);
    float finalDelta = max(maxDelta.x, maxDelta.y);

    // Local contrast adaptation:
    edges.xy *= step( finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy );

    vec4 subsampleIndices = vec4(0.0); // Just pass zero for SMAA 1x, see @SUBSAMPLE_INDICES.
  // subsampleIndices = vec4(1.0, 1.0, 1.0, 0.0);
    weights = vec4(0.0, 0.0, 0.0, 0.0);
  vec2 e = texture2D(u_DiffuseMap, v_TexCoords).rg;

  if (e.g > 0.0) { // Edge at north

    #if !defined(SMAA_DISABLE_DIAG_DETECTION)
    // Diagonals have both north and west edges, so searching for them in
    // one of the boundaries is enough.
    weights.rg = SMAACalculateDiagWeights(u_DiffuseMap, u_DiffuseMap, v_TexCoords, e, subsampleIndices);

    // We give priority to diagonals, so if we find a diagonal we skip
    // horizontal/vertical processing.
    if (weights.r == -weights.g) { // weights.r + weights.g == 0.0
    #endif

    vec2 d;

    // Find the distance to the left:
    vec3 coords;
    coords.x = SMAASearchXLeft(u_DiffuseMap, u_DiffuseMap, v_Offset[0].xy, v_Offset[2].x);
    coords.y = v_Offset[1].y; // v_Offset[1].y = v_TexCoords.y - 0.25 * SMAA_RT_METRICS.y (@CROSSING_OFFSET)
    d.x = coords.x;

    // Now fetch the left crossing edges, two at a time using bilinear
    // filtering. Sampling at -0.25 (see @CROSSING_OFFSET) enables to
    // discern what value each edge has:
    float e1 = texture2D(u_DiffuseMap, coords.xy).r; // LinearSampler

    // Find the distance to the right:
    coords.z = SMAASearchXRight(u_DiffuseMap, u_DiffuseMap, v_Offset[0].zw, v_Offset[2].y);
    d.y = coords.z;

    // We want the distances to be in pixel units (doing this here allow to
    // better interleave arithmetic and memory accesses):
    d = abs(round(mad(SMAA_RT_METRICS.zz, d, -v_PixCoord.xx)));

    // SMAAArea below needs a sqrt, as the areas texture is compressed
    // quadratically:
    vec2 sqrt_d = sqrt(d);

    // Fetch the right crossing edges:
    float e2 = SMAASampleLevelZeroOffset(u_DiffuseMap, coords.zy, vec2(1, 0)).r;

    // Ok, we know how this pattern looks like, now it is time for getting
    // the actual area:
    weights.rg = SMAAArea(u_DiffuseMap, sqrt_d, e1, e2, subsampleIndices.y);

    // Fix corners:
    coords.y = v_TexCoords.y;
    SMAADetectHorizontalCornerPattern(u_DiffuseMap, weights.rg, coords.xyzy, d);

    #if !defined(SMAA_DISABLE_DIAG_DETECTION)
    } else
    e.r = 0.0; // Skip vertical processing.
    #endif
  }

  if (e.r > 0.0) { // Edge at west
    vec2 d;
    // Find the distance to the top:
    vec3 coords;
    coords.y = SMAASearchYUp(u_DiffuseMap, u_DiffuseMap, v_Offset[1].xy, v_Offset[2].z);
    coords.x = v_Offset[0].x; // v_Offset[1].x = v_TexCoords.x - 0.25 * SMAA_RT_METRICS.x;
    d.x = coords.y;

    // Fetch the top crossing edges:
    float e1 = texture2D(u_DiffuseMap, coords.xy).g; // LinearSampler

    // Find the distance to the bottom:
    coords.z = SMAASearchYDown(u_DiffuseMap, u_DiffuseMap, v_Offset[1].zw, v_Offset[2].w);
    d.y = coords.z;

    // We want the distances to be in pixel units:
    d = abs(round(mad(SMAA_RT_METRICS.ww, d, -v_PixCoord.yy)));

    // SMAAArea below needs a sqrt, as the areas texture is compressed
    // quadratically:
    vec2 sqrt_d = sqrt(d);

    // Fetch the bottom crossing edges:
    float e2 = SMAASampleLevelZeroOffset(u_DiffuseMap, coords.xz, vec2(0, 1)).g;

    // Get the area for this direction:
    weights.ba = SMAAArea(u_DiffuseMap, sqrt_d, e1, e2, subsampleIndices.x);

    // Fix corners:
    coords.x = v_TexCoords.x;
    SMAADetectVerticalCornerPattern(u_DiffuseMap, weights.ba, coords.xyxz, d);
  }
#endif

    a_Color = v_Color * sharpenImage( u_DiffuseMap, v_TexCoords );
//	a_Color = v_Color * texture2D( u_DiffuseMap, v_TexCoords );

    a_Color.rgb = pow( a_Color.rgb, vec3( 1.0 / u_GammaAmount ) );
}