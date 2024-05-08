#ifndef _N_MATH_
#define _N_MATH_

#pragma once

#ifdef __cplusplus
#include <glm/glm.hpp>
#endif
#include "gln_files.h"

// bounding boxes
typedef struct {
	vec3_t mins;
	vec3_t maxs;
} bbox_t;

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

float Q_rsqrt( float f );		// reciprocal square root

float N_log2f( float f );
float N_exp2f( float f );

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

signed char ClampChar( int i );
signed char ClampCharMove( int i );
signed short ClampShort( int i );

float RadiusFromBounds( const bbox_t *bounds );
void ClearBounds( bbox_t *bounds );

vec_t VectorNormalize2( const vec3_t v, vec3_t out );

float disBetweenOBJ( const vec3_t src, const vec3_t tar );

#ifdef __cplusplus
unsigned disBetweenOBJ( const uvec3_t src, const uvec3_t tar );
int disBetweenOBJ( const ivec3_t src, const ivec3_t tar );

float disBetweenOBJ( const glm::vec2& src, const glm::vec2& tar );
unsigned disBetweenOBJ( const glm::uvec2& src, const glm::uvec2& tar );
int disBetweenOBJ( const glm::ivec2& src, const glm::ivec2& tar );

float disBetweenOBJ( const glm::vec3& src, const glm::vec3& tar );
unsigned disBetweenOBJ( const glm::uvec3& src, const glm::uvec3& tar );
int disBetweenOBJ( const glm::ivec3& src, const glm::ivec3& tar );
#endif

dirtype_t Angle2Dir( float angle );
float Dir2Angle( dirtype_t dir );
dirtype_t DirFromPoint( const vec3_t v );

qboolean BoundsIntersect( const bbox_t *a, const bbox_t *b );
qboolean BoundsIntersectSphere( const bbox_t *bounds,
		const vec3_t origin, vec_t radius );
qboolean BoundsIntersectPoint( const bbox_t *bounds,
		const vec3_t origin );

GDR_INLINE int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

GDR_INLINE vec_t VectorLength( const vec3_t v ) {
	return (vec_t)sqrtf (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

GDR_INLINE vec_t VectorLengthSquared( const vec3_t v ) {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

GDR_INLINE vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return VectorLength( v );
}

GDR_INLINE vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
GDR_INLINE void VectorNormalizeFast( vec3_t v )
{
	float ilength;

	ilength = Q_rsqrt( DotProduct( v, v ) );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

GDR_INLINE void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

GDR_INLINE void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

// just in case you don't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

float NormalizeColor( const vec3_t in, vec3_t out );
float RadiusFromBounds( const bbox_t *bounds );
void ClearBounds( bbox_t *bounds );
void AddPointToBounds( const vec3_t v, bbox_t *bounds );

void vectoangles( const vec3_t value1, vec3_t angles);
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );
vec_t VectorNormalize( vec3_t v );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

int N_log2(int val);

float N_acos(float c);
float N_fabs( float f );

int N_rand( int *seed );
float N_random( int *seed );
float N_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

int N_isnan( float x );

#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif
#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#ifndef SGN
#define SGN(x) (((x) >= 0) ? !!(x) : -1)
#endif

#ifndef CLAMP
#define CLAMP(a,b,c) MIN(MAX((a),(b)),(c))
#endif

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

#define	MAX_INT			0x7fffffff
#define	MIN_INT			(-MAX_INT-1)

#define MAX_USHORT      ((uint16_t)(~0))
#define	MAX_UINT		((uint32_t)(~0))
#define MAX_ULONG       ((uint64_t)(~0))

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef ABS
#define ABS( x ) ( (x) ? (x) : -(x) )
#endif

#endif
