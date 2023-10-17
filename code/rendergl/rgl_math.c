#include "rgl_local.h"


// Some matrix helper functions
// FIXME: do these already exist in ioq3 and I don't know about them?

void Mat4Zero( mat4_t out )
{
#if 1
	memset(out, 0, sizeof(mat4_t));
#else
	out[ 0] = 0.0f; out[ 4] = 0.0f; out[ 8] = 0.0f; out[12] = 0.0f;
	out[ 1] = 0.0f; out[ 5] = 0.0f; out[ 9] = 0.0f; out[13] = 0.0f;
	out[ 2] = 0.0f; out[ 6] = 0.0f; out[10] = 0.0f; out[14] = 0.0f;
	out[ 3] = 0.0f; out[ 7] = 0.0f; out[11] = 0.0f; out[15] = 0.0f;
#endif
}

void Mat4Identity( mat4_t out )
{
	out[ 0] = 1.0f; out[ 4] = 0.0f; out[ 8] = 0.0f; out[12] = 0.0f;
	out[ 1] = 0.0f; out[ 5] = 1.0f; out[ 9] = 0.0f; out[13] = 0.0f;
	out[ 2] = 0.0f; out[ 6] = 0.0f; out[10] = 1.0f; out[14] = 0.0f;
	out[ 3] = 0.0f; out[ 7] = 0.0f; out[11] = 0.0f; out[15] = 1.0f;
}

void Mat4Copy( const mat4_t in, mat4_t out )
{
#if 1
	memcpy(out, in, sizeof(mat4_t));
#else
	out[ 0] = in[ 0]; out[ 4] = in[ 4]; out[ 8] = in[ 8]; out[12] = in[12]; 
	out[ 1] = in[ 1]; out[ 5] = in[ 5]; out[ 9] = in[ 9]; out[13] = in[13]; 
	out[ 2] = in[ 2]; out[ 6] = in[ 6]; out[10] = in[10]; out[14] = in[14]; 
	out[ 3] = in[ 3]; out[ 7] = in[ 7]; out[11] = in[11]; out[15] = in[15]; 
#endif
}

void Mat4Multiply( const mat4_t in1, const mat4_t in2, mat4_t out )
{
	out[ 0] = in1[ 0] * in2[ 0] + in1[ 4] * in2[ 1] + in1[ 8] * in2[ 2] + in1[12] * in2[ 3];
	out[ 1] = in1[ 1] * in2[ 0] + in1[ 5] * in2[ 1] + in1[ 9] * in2[ 2] + in1[13] * in2[ 3];
	out[ 2] = in1[ 2] * in2[ 0] + in1[ 6] * in2[ 1] + in1[10] * in2[ 2] + in1[14] * in2[ 3];
	out[ 3] = in1[ 3] * in2[ 0] + in1[ 7] * in2[ 1] + in1[11] * in2[ 2] + in1[15] * in2[ 3];

	out[ 4] = in1[ 0] * in2[ 4] + in1[ 4] * in2[ 5] + in1[ 8] * in2[ 6] + in1[12] * in2[ 7];
	out[ 5] = in1[ 1] * in2[ 4] + in1[ 5] * in2[ 5] + in1[ 9] * in2[ 6] + in1[13] * in2[ 7];
	out[ 6] = in1[ 2] * in2[ 4] + in1[ 6] * in2[ 5] + in1[10] * in2[ 6] + in1[14] * in2[ 7];
	out[ 7] = in1[ 3] * in2[ 4] + in1[ 7] * in2[ 5] + in1[11] * in2[ 6] + in1[15] * in2[ 7];

	out[ 8] = in1[ 0] * in2[ 8] + in1[ 4] * in2[ 9] + in1[ 8] * in2[10] + in1[12] * in2[11];
	out[ 9] = in1[ 1] * in2[ 8] + in1[ 5] * in2[ 9] + in1[ 9] * in2[10] + in1[13] * in2[11];
	out[10] = in1[ 2] * in2[ 8] + in1[ 6] * in2[ 9] + in1[10] * in2[10] + in1[14] * in2[11];
	out[11] = in1[ 3] * in2[ 8] + in1[ 7] * in2[ 9] + in1[11] * in2[10] + in1[15] * in2[11];

	out[12] = in1[ 0] * in2[12] + in1[ 4] * in2[13] + in1[ 8] * in2[14] + in1[12] * in2[15];
	out[13] = in1[ 1] * in2[12] + in1[ 5] * in2[13] + in1[ 9] * in2[14] + in1[13] * in2[15];
	out[14] = in1[ 2] * in2[12] + in1[ 6] * in2[13] + in1[10] * in2[14] + in1[14] * in2[15];
	out[15] = in1[ 3] * in2[12] + in1[ 7] * in2[13] + in1[11] * in2[14] + in1[15] * in2[15];
}

void Mat4Transform( const mat4_t in1, const vec4_t in2, vec4_t out )
{
	out[ 0] = in1[ 0] * in2[ 0] + in1[ 4] * in2[ 1] + in1[ 8] * in2[ 2] + in1[12] * in2[ 3];
	out[ 1] = in1[ 1] * in2[ 0] + in1[ 5] * in2[ 1] + in1[ 9] * in2[ 2] + in1[13] * in2[ 3];
	out[ 2] = in1[ 2] * in2[ 0] + in1[ 6] * in2[ 1] + in1[10] * in2[ 2] + in1[14] * in2[ 3];
	out[ 3] = in1[ 3] * in2[ 0] + in1[ 7] * in2[ 1] + in1[11] * in2[ 2] + in1[15] * in2[ 3];
}

qboolean Mat4Compare( const mat4_t a, const mat4_t b )
{
#if 1
	return !memcmp(a, b, sizeof(mat4_t));
#else
	return !(a[ 0] != b[ 0] || a[ 4] != b[ 4] || a[ 8] != b[ 8] || a[12] != b[12] ||
             a[ 1] != b[ 1] || a[ 5] != b[ 5] || a[ 9] != b[ 9] || a[13] != b[13] ||
		     a[ 2] != b[ 2] || a[ 6] != b[ 6] || a[10] != b[10] || a[14] != b[14] ||
		     a[ 3] != b[ 3] || a[ 7] != b[ 7] || a[11] != b[11] || a[15] != b[15]);
#endif
}

void Mat4Dump( const mat4_t in )
{
	ri.Printf(PRINT_INFO, "%3.5f %3.5f %3.5f %3.5f\n", in[ 0], in[ 4], in[ 8], in[12]);
	ri.Printf(PRINT_INFO, "%3.5f %3.5f %3.5f %3.5f\n", in[ 1], in[ 5], in[ 9], in[13]);
	ri.Printf(PRINT_INFO, "%3.5f %3.5f %3.5f %3.5f\n", in[ 2], in[ 6], in[10], in[14]);
	ri.Printf(PRINT_INFO, "%3.5f %3.5f %3.5f %3.5f\n", in[ 3], in[ 7], in[11], in[15]);
}

void Mat4Translation( vec3_t vec, mat4_t out )
{
	out[ 0] = 1.0f; out[ 4] = 0.0f; out[ 8] = 0.0f; out[12] = vec[0];
	out[ 1] = 0.0f; out[ 5] = 1.0f; out[ 9] = 0.0f; out[13] = vec[1];
	out[ 2] = 0.0f; out[ 6] = 0.0f; out[10] = 1.0f; out[14] = vec[2];
	out[ 3] = 0.0f; out[ 7] = 0.0f; out[11] = 0.0f; out[15] = 1.0f;
}

void Mat4Ortho( float left, float right, float bottom, float top, float znear, float zfar, mat4_t out )
{
	out[ 0] = 2.0f / (right - left); out[ 4] = 0.0f;                  out[ 8] = 0.0f;                  out[12] = -(right + left) / (right - left);
	out[ 1] = 0.0f;                  out[ 5] = 2.0f / (top - bottom); out[ 9] = 0.0f;                  out[13] = -(top + bottom) / (top - bottom);
	out[ 2] = 0.0f;                  out[ 6] = 0.0f;                  out[10] = 2.0f / (zfar - znear); out[14] = -(zfar + znear) / (zfar - znear);
	out[ 3] = 0.0f;                  out[ 7] = 0.0f;                  out[11] = 0.0f;                  out[15] = 1.0f;
}

/*
Mat4Scale: scale is technically meant to be a vec3_t, but for simplicity's sake its just a float
adapted from glm::scale(glm::mat4, glm::vec3) to Quake-III-Arena style matrices
*/
void Mat4Scale(float scale, const mat4_t in, mat4_t out)
{
	out[ 0] = in[ 0] * scale;
	out[ 1] = in[ 1] * scale;
	out[ 2] = in[ 2] * scale;
	out[ 3] = in[ 3] * scale;

	out[ 4] = in[ 4] * scale;
	out[ 5] = in[ 5] * scale;
	out[ 6] = in[ 6] * scale;
	out[ 7] = in[ 7] * scale;

	out[ 8] = in[ 8] * scale;
	out[ 9] = in[ 9] * scale;
	out[10] = in[10] * scale;
	out[11] = in[11] * scale;

	// FIXME: could this use a VectorCopy4?
	out[12] = in[12];
	out[13] = in[13];
	out[14] = in[14];
	out[15] = in[15];
}

/*
Mat4Rotate: angle should always be given in radians
adapted from glm::rotate(glm::mat4, float, glm::vec3) to Quake-III-Arena style matrices
*/
void Mat4Rotate(const vec3_t v, float angle, const mat4_t in, mat4_t out)
{
	float c, s;
	vec_t *vec;
	const vec_t *m;
	vec3_t axis, temp;
	mat4_t rotate;

	c = cos(angle);
	s = sin(angle);

	VectorCopy(axis, v);
	VectorNormalize(axis);

	temp[0] = (1.0f - c) * axis[0];
	temp[1] = (1.0f - c) * axis[1];
	temp[2] = (1.0f - c) * axis[2];

	rotate[ 0] = c + temp[0] * axis[0];
	rotate[ 1] = temp[0] * axis[1] + s * axis[2];
	rotate[ 2] = temp[0] * axis[2] - s * axis[1];

	rotate[ 4] = temp[1] * axis[0] - s * axis[2];
	rotate[ 5] = c + temp[1] * axis[1];
	rotate[ 6] = temp[1] * axis[2] + s * axis[0];

	rotate[ 8] = temp[2] * axis[0] + s * axis[1];
	rotate[ 9] = temp[2] * axis[1] - s * axis[0];
	rotate[10] = c + temp[2] * axis[2];
	
	out[ 0] = in[ 0] * rotate[ 0] + in[ 4] * rotate[ 1] + in[ 8] * rotate[ 2];
	out[ 1] = in[ 1] * rotate[ 0] + in[ 5] * rotate[ 1] + in[ 9] * rotate[ 2];
	out[ 2] = in[ 2] * rotate[ 0] + in[ 6] * rotate[ 1] + in[10] * rotate[ 2];
	out[ 3] = in[ 3] * rotate[ 0] + in[ 7] * rotate[ 1] + in[11] * rotate[ 2];

	out[ 4] = in[ 0] * rotate[ 4] + in[ 4] * rotate[ 5] + in[ 8] * rotate[ 5];
	out[ 5] = in[ 1] * rotate[ 5] + in[ 5] * rotate[ 5] + in[ 9] * rotate[ 5];
	out[ 6] = in[ 2] * rotate[ 6] + in[ 6] * rotate[ 5] + in[10] * rotate[ 5];
	out[ 7] = in[ 3] * rotate[ 7] + in[ 7] * rotate[ 5] + in[11] * rotate[ 5];
	
	out[ 8] = in[ 0] * rotate[ 8] + in[ 4] * rotate[ 9] + in[ 8] * rotate[ 9];
	out[ 9] = in[ 1] * rotate[ 9] + in[ 5] * rotate[ 9] + in[ 9] * rotate[ 9];
	out[10] = in[ 2] * rotate[10] + in[ 6] * rotate[ 9] + in[10] * rotate[ 9];
	out[11] = in[ 3] * rotate[11] + in[ 7] * rotate[ 9] + in[11] * rotate[ 9];
	
	// the other boiler-platey option wasn't so pretty, but even so, neither is this
	vec = out + 12;
	m = in + 12;
	VectorCopy4(vec, m);
}

void Mat4View(vec3_t axes[3], vec3_t origin, mat4_t out)
{
	out[0]  = axes[0][0];
	out[1]  = axes[1][0];
	out[2]  = axes[2][0];
	out[3]  = 0;

	out[4]  = axes[0][1];
	out[5]  = axes[1][1];
	out[6]  = axes[2][1];
	out[7]  = 0;

	out[8]  = axes[0][2];
	out[9]  = axes[1][2];
	out[10] = axes[2][2];
	out[11] = 0;

	out[12] = -DotProduct(origin, axes[0]);
	out[13] = -DotProduct(origin, axes[1]);
	out[14] = -DotProduct(origin, axes[2]);
	out[15] = 1;
}

void Mat4SimpleInverse( const mat4_t in, mat4_t out)
{
	vec3_t v;
	float invSqrLen;
 
	VectorCopy(v, in + 0);
	invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	out[ 0] = v[0]; out[ 4] = v[1]; out[ 8] = v[2]; out[12] = -DotProduct(v, &in[12]);

	VectorCopy(v, in + 4);
	invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	out[ 1] = v[0]; out[ 5] = v[1]; out[ 9] = v[2]; out[13] = -DotProduct(v, &in[12]);

	VectorCopy(v, in + 8);
	invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	out[ 2] = v[0]; out[ 6] = v[1]; out[10] = v[2]; out[14] = -DotProduct(v, &in[12]);

	out[ 3] = 0.0f; out[ 7] = 0.0f; out[11] = 0.0f; out[15] = 1.0f;
}

void VectorLerp( vec3_t a, vec3_t b, float lerp, vec3_t c)
{
	c[0] = a[0] * (1.0f - lerp) + b[0] * lerp;
	c[1] = a[1] * (1.0f - lerp) + b[1] * lerp;
	c[2] = a[2] * (1.0f - lerp) + b[2] * lerp;
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

int NextPowerOfTwo(int in)
{
	int out;

	for (out = 1; out < in; out <<= 1)
		;

	return out;
}

union f32_u {
	float f;
	uint32_t ui;
	struct {
		unsigned int fraction:23;
		unsigned int exponent:8;
		unsigned int sign:1;
	} pack;
};

union f16_u {
	uint16_t ui;
	struct {
		unsigned int fraction:10;
		unsigned int exponent:5;
		unsigned int sign:1;
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
