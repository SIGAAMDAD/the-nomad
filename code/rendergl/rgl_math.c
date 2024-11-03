#include "rgl_local.h"

/*
key:

0: [0][0]
1: [0][1]
2: [0][2]
3: [0][3]

4: [1][0]
5: [1][1]
6: [1][2]
7: [1][3]

8: [2][0]
9: [2][1]
10: [2][2]
11: [2][3]

12: [3][0]
13: [3][1]
14: [3][2]
15: [3][3]

*/

// Some matrix helper functions
// FIXME: do these already exist in ioq3 and I don't know about them?

void Mat4Zero( mat4_t out )
{
	memset( out, 0, sizeof( mat4_t ) );
}

void Mat4Identity( mat4_t out )
{
	out[0][0] = 1.0f; out[1][0] = 0.0f; out[2][0] = 0.0f; out[3][0] = 0.0f;
	out[0][1] = 0.0f; out[1][1] = 1.0f; out[2][1] = 0.0f; out[3][1] = 0.0f;
	out[0][2] = 0.0f; out[1][2] = 0.0f; out[2][2] = 1.0f; out[3][2] = 0.0f;
	out[0][3] = 0.0f; out[1][3] = 0.0f; out[2][3] = 0.0f; out[3][3] = 1.0f;
}

void Mat4Copy( const mat4_t in, mat4_t out )
{
	memcpy( out, in, sizeof( mat4_t ) );
}

void Mat4Multiply( const mat4_t in1, const mat4_t in2, mat4_t out )
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[1][0] * in2[0][1] + in1[2][0] * in2[0][2] + in1[3][0] * in2[0][3];
	out[0][1] = in1[0][1] * in2[0][0] + in1[1][1] * in2[0][1] + in1[2][1] * in2[0][2] + in1[3][1] * in2[0][3];
	out[0][2] = in1[0][2] * in2[0][0] + in1[1][2] * in2[0][1] + in1[2][2] * in2[0][2] + in1[3][2] * in2[0][3];
	out[0][3] = in1[0][3] * in2[0][0] + in1[1][3] * in2[0][1] + in1[2][3] * in2[0][2] + in1[3][3] * in2[0][3];

	out[1][0] = in1[0][0] * in2[1][0] + in1[1][0] * in2[1][1] + in1[2][0] * in2[1][2] + in1[3][0] * in2[1][3];
	out[1][1] = in1[0][1] * in2[1][0] + in1[1][1] * in2[1][1] + in1[2][1] * in2[1][2] + in1[3][1] * in2[1][3];
	out[1][2] = in1[0][2] * in2[1][0] + in1[1][2] * in2[1][1] + in1[2][2] * in2[1][2] + in1[3][2] * in2[1][3];
	out[1][3] = in1[0][3] * in2[1][0] + in1[1][3] * in2[1][1] + in1[2][3] * in2[1][2] + in1[3][3] * in2[1][3];

	out[2][0] = in1[0][0] * in2[2][0] + in1[1][0] * in2[2][1] + in1[2][0] * in2[2][2] + in1[3][0] * in2[2][3];
	out[2][1] = in1[0][1] * in2[2][0] + in1[1][1] * in2[2][1] + in1[2][1] * in2[2][2] + in1[3][1] * in2[2][3];
	out[2][2] = in1[0][2] * in2[2][0] + in1[1][2] * in2[2][1] + in1[2][2] * in2[2][2] + in1[3][2] * in2[2][3];
	out[2][3] = in1[0][3] * in2[2][0] + in1[1][3] * in2[2][1] + in1[2][3] * in2[2][2] + in1[3][3] * in2[2][3];

	out[3][0] = in1[0][0] * in2[3][0] + in1[1][0] * in2[3][1] + in1[2][0] * in2[3][2] + in1[3][0] * in2[3][3];
	out[3][1] = in1[0][1] * in2[3][0] + in1[1][1] * in2[3][1] + in1[2][1] * in2[3][2] + in1[3][1] * in2[3][3];
	out[3][2] = in1[0][2] * in2[3][0] + in1[1][2] * in2[3][1] + in1[2][2] * in2[3][2] + in1[3][2] * in2[3][3];
	out[3][3] = in1[0][3] * in2[3][0] + in1[1][3] * in2[3][1] + in1[2][3] * in2[3][2] + in1[3][3] * in2[3][3];
}

void Mat4Transform( const mat4_t in1, const vec4_t in2, vec4_t out )
{
	out[0] = in1[0][0] * in2[0] + in1[1][0] * in2[1] + in1[2][0] * in2[2] + in1[3][0] * in2[3];
	out[1] = in1[0][1] * in2[0] + in1[1][1] * in2[1] + in1[2][1] * in2[2] + in1[3][1] * in2[3];
	out[2] = in1[0][2] * in2[0] + in1[1][2] * in2[1] + in1[2][2] * in2[2] + in1[3][2] * in2[3];
	out[3] = in1[0][3] * in2[0] + in1[1][3] * in2[1] + in1[2][3] * in2[2] + in1[3][3] * in2[3];
}

//
// Mat4Compare: returns qfalse if not equal, qtrue otherwise
//
qboolean Mat4Compare( const mat4_t a, const mat4_t b )
{
	return memcmp( a, b, sizeof( mat4_t ) ) == 0;
}

void Mat4Dump( const mat4_t in )
{
	ri.Printf( PRINT_INFO, "%5.5f %5.5f %5.5f %5.5f\n", in[0][0], in[1][0], in[2][0], in[3][0] );
	ri.Printf( PRINT_INFO, "%5.5f %5.5f %5.5f %5.5f\n", in[0][1], in[1][1], in[2][1], in[3][1] );
	ri.Printf( PRINT_INFO, "%5.5f %5.5f %5.5f %5.5f\n", in[0][2], in[1][2], in[2][2], in[3][2] );
	ri.Printf( PRINT_INFO, "%5.5f %5.5f %5.5f %5.5f\n", in[0][3], in[1][3], in[2][3], in[3][3] );
}

void Mat4Translation( vec3_t vec, mat4_t out )
{
	out[0][0] = 1.0f; out[1][0] = 0.0f; out[2][0] = 0.0f; out[3][0] = vec[0];
	out[0][1] = 0.0f; out[1][1] = 1.0f; out[2][1] = 0.0f; out[3][1] = vec[1];
	out[0][2] = 0.0f; out[1][2] = 0.0f; out[2][2] = 1.0f; out[3][2] = vec[2];
	out[0][3] = 0.0f; out[1][3] = 0.0f; out[2][3] = 0.0f; out[3][3] = 1.0f;
}

void Mat4Ortho( float left, float right, float bottom, float top, float znear, float zfar, mat4_t out )
{
	out[0][0] = 2.0f / (right - left); out[1][0] = 0.0f;                  out[2][0] = 0.0f;                  out[3][0] = -(right + left) / (right - left);
	out[0][1] = 0.0f;                  out[1][1] = 2.0f / (top - bottom); out[2][1] = 0.0f;                  out[3][1] = -(top + bottom) / (top - bottom);
	out[0][2] = 0.0f;                  out[1][2] = 0.0f;                  out[2][2] = 2.0f / (zfar - znear); out[3][2] = -(zfar + znear) / (zfar - znear);
	out[0][3] = 0.0f;                  out[1][3] = 0.0f;                  out[2][3] = 0.0f;                  out[3][3] = 1.0f;
}

//
// Mat4Scale: scale is technically meant to be a vec3_t, but for simplicity's sake its just a float
// adapted from glm::scale(glm::mat4, glm::vec3) to Quake-III-Arena style matrices
//
void Mat4Scale(float scale, const mat4_t in, mat4_t out)
{
	VectorScale4( in[0], scale, out[0] );
	VectorScale4( in[1], scale, out[1] );
	VectorScale4( in[2], scale, out[2] );
	VectorCopy4( out[3], in[3] );
}

//
// Mat4Rotate: angle should always be given in radians
// adapted from glm::rotate(glm::mat4, float, glm::vec3) to Quake-III-Arena style matrices
//
void Mat4Rotate(const vec3_t v, float angle, const mat4_t in, mat4_t out)
{
	float c, s;
	vec_t *vec;
	const vec_t *m;
	vec3_t axis, temp;
	mat4_t rotate;

	c = cos( angle );
	s = sin( angle );

	VectorCopy( axis, v );
	VectorNormalize( axis );

	temp[0] = ( 1.0f - c ) * axis[0];
	temp[1] = ( 1.0f - c ) * axis[1];
	temp[2] = ( 1.0f - c ) * axis[2];

	rotate[0][0] = c + temp[0] * axis[0];
	rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	rotate[1][1] = c + temp[1] * axis[1];
	rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	rotate[2][2] = c + temp[2] * axis[2];
	
	out[0][0] = in[0][0] * rotate[0][0] + in[1][0] * rotate[0][1] + in[2][0] * rotate[0][1];
	out[0][1] = in[0][1] * rotate[0][0] + in[1][1] * rotate[0][1] + in[2][1] * rotate[0][1];
	out[0][2] = in[0][2] * rotate[0][0] + in[1][2] * rotate[0][1] + in[2][2] * rotate[0][1];
	out[0][3] = in[0][3] * rotate[0][0] + in[1][3] * rotate[0][1] + in[2][3] * rotate[0][1];

	out[1][0] = in[0][0] * rotate[1][0] + in[1][0] * rotate[1][1] + in[2][0] * rotate[1][1];
	out[1][1] = in[0][1] * rotate[1][1] + in[1][1] * rotate[1][1] + in[2][1] * rotate[1][1];
	out[1][2] = in[0][2] * rotate[1][2] + in[1][2] * rotate[1][1] + in[2][2] * rotate[1][1];
	out[1][3] = in[0][3] * rotate[1][3] + in[1][3] * rotate[1][1] + in[2][3] * rotate[1][1];
	
	out[2][0] = in[0][0] * rotate[2][0] + in[1][0] * rotate[2][0] + in[2][0] * rotate[2][1];
	out[2][1] = in[0][1] * rotate[2][1] + in[1][1] * rotate[2][1] + in[2][1] * rotate[2][1];
	out[2][2] = in[0][2] * rotate[2][2] + in[1][2] * rotate[2][2] + in[2][2] * rotate[2][1];
	out[2][3] = in[0][3] * rotate[2][3] + in[1][3] * rotate[2][3] + in[2][3] * rotate[2][1];
	
	// the other boiler-platey option wasn't so pretty, but even so, neither is this
	vec = out + 12;
	m = in + 12;
	VectorCopy4( vec, m );
}

void Mat4View(vec3_t axes[3], vec3_t origin, mat4_t out)
{
	out[0][0] = axes[0][0];
	out[0][1] = axes[1][0];
	out[0][2] = axes[2][0];
	out[0][3] = 0;

	out[1][0] = axes[0][1];
	out[1][1] = axes[1][1];
	out[1][2] = axes[2][1];
	out[1][3] = 0;

	out[2][0] = axes[0][2];
	out[2][1] = axes[1][2];
	out[2][2] = axes[2][2];
	out[2][3] = 0;

	out[3][0] = -DotProduct( origin, axes[0] );
	out[3][1] = -DotProduct( origin, axes[1] );
	out[3][2] = -DotProduct( origin, axes[2] );
	out[3][3] = 1;
}

void Mat4SimpleInverse( const mat4_t in, mat4_t out )
{
	vec3_t v;
	float invSqrLen;
 
	VectorCopy( v, in[0] );
	invSqrLen = 1.0f / DotProduct( v, v );
	VectorScale( v, invSqrLen, v );
	out[0][0] = v[0];
	out[1][0] = v[1];
	out[2][0] = v[2];
	out[3][0] = -DotProduct( v, in[3] );

	VectorCopy( v, in[1] );
	invSqrLen = 1.0f / DotProduct( v, v );
	VectorScale( v, invSqrLen, v );
	out[0][1] = v[0];
	out[1][1] = v[1];
	out[2][1] = v[2];
	out[3][1] = -DotProduct( v, in[12] );

	VectorCopy( v, in[2] );
	invSqrLen = 1.0f / DotProduct( v, v );
	VectorScale( v, invSqrLen, v );
	out[0][2] = v[0];
	out[1][2] = v[1];
	out[2][2] = v[2];
	out[3][2] = -DotProduct( v, in[12] );

	out[0][3] = 0.0f;
	out[1][3] = 0.0f;
	out[2][3] = 0.0f;
	out[3][3] = 1.0f;
}

void VectorLerp( vec3_t a, vec3_t b, float lerp, vec3_t c )
{
	c[0] = a[0] * ( 1.0f - lerp ) + b[0] * lerp;
	c[1] = a[1] * ( 1.0f - lerp ) + b[1] * lerp;
	c[2] = a[2] * ( 1.0f - lerp ) + b[2] * lerp;
}

qboolean SpheresIntersect(vec3_t origin1, float radius1, vec3_t origin2, float radius2)
{
	float radiusSum = radius1 + radius2;
	vec3_t diff;
	
	VectorSubtract(origin1, origin2, diff);

	if (DotProduct(diff, diff) <= radiusSum * radiusSum)
	{
		return qtrue;
	}

	return qfalse;
}

void BoundingSphereOfSpheres(vec3_t origin1, float radius1, vec3_t origin2, float radius2, vec3_t origin3, float *radius3)
{
	vec3_t diff;

	VectorScale(origin1, 0.5f, origin3);
	VectorMA(origin3, 0.5f, origin2, origin3);

	VectorSubtract(origin1, origin2, diff);
	*radius3 = VectorLength(diff) * 0.5f + MAX(radius1, radius2);
}

int32_t NextPowerOfTwo( int32_t in )
{
	int32_t out;

	for ( out = 1; out < in; out <<= 1 )
		;

	return out;
}

union f32_u {
	float f;
	uint32_t ui;
	struct {
		uint32_t fraction:23;
		uint32_t exponent:8;
		uint32_t sign:1;
	} pack;
};

union f16_u {
	uint16_t ui;
	struct {
		uint32_t fraction:10;
		uint32_t exponent:5;
		uint32_t sign:1;
	} pack;
};

uint16_t FloatToHalf(float in)
{
	union f32_u f32;
	union f16_u f16;

	f32.f = in;

	f16.pack.exponent = CLAMP((int)(f32.pack.exponent) - 112, 0, 31);
	f16.pack.fraction = f32.pack.fraction >> 13;
	f16.pack.sign     = f32.pack.sign;

	return f16.ui;
}

float HalfToFloat(uint16_t in)
{
	union f32_u f32;
	union f16_u f16;

	f16.ui = in;

	f32.pack.exponent = (int)(f16.pack.exponent) + 112;
	f32.pack.fraction = f16.pack.fraction << 13;
	f32.pack.sign = f16.pack.sign;

	return f32.f;
}
