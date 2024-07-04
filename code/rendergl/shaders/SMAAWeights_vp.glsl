#define mad( a, b, c ) ( a * b + c )

#if defined(SMAA_PRESET_LOW)
#define SMAA_MAX_SEARCH_STEPS 4
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_MAX_SEARCH_STEPS 8
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_MAX_SEARCH_STEPS 16
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_MAX_SEARCH_STEPS 32
#endif

#if !defined(SMAA_MAX_SEARCH_STEPS)
#define SMAA_MAX_SEARCH_STEPS 16
#endif

in vec2 a_Position;

uniform vec2 u_ScreenSize;

out vec2 v_TexCoords;
out vec2 v_PixCoord;
out vec4 v_Offset[3];

void main() {
    vec4 SMAA_RT_METRICS = vec4( 1.0 / u_ScreenSize.x, 1.0 / u_ScreenSize.y, u_ScreenSize.x, u_ScreenSize.y );

	v_TexCoords = vec2( ( a_Position + 1.0 ) / 2.0 );
    v_PixCoord = v_TexCoords * SMAA_RT_METRICS.zw;

    // We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
    v_Offset[0] = mad( SMAA_RT_METRICS.xyxy, vec4( -0.25, -0.125,  1.25, -0.125 ), v_TexCoords.xyxy );
    v_Offset[1] = mad( SMAA_RT_METRICS.xyxy, vec4( -0.125, -0.25, -0.125,  1.25 ), v_TexCoords.xyxy );

    // And these for the searches, they indicate the ends of the loops:
    v_Offset[2] = mad(
        SMAA_RT_METRICS.xxyy,
        vec4( -2.0, 2.0, -2.0, 2.0 ) * float( SMAA_MAX_SEARCH_STEPS ),
        vec4( v_Offset[0].xz, v_Offset[1].yw )
    );

    gl_Position = vec4( a_Position, 0.0, 1.0 );
}