#ifndef GDR_DLLCOMPILE
#include "n_shared.h"
#else
#include "../engine/n_shared.h"
#endif

#if defined(__SSE2__) || defined(_MSC_SSE2_)
#define USING_SSE2
#ifdef _MSC
#include <intrin.h>
#else
#include <immintrin.h>
#include <xmmintrin.h>
#endif
#endif

const vec2_t vec2_origin = {0, 0};
const vec3_t vec3_origin = {0, 0};

const vec4_t		colorBlack	= {0, 0, 0, 1};
const vec4_t		colorRed	= {1, 0, 0, 1};
const vec4_t		colorGreen	= {0, 1, 0, 1};
const vec4_t		colorBlue	= {0, 0, 1, 1};
const vec4_t		colorYellow	= {1, 1, 0, 1};
const vec4_t		colorMagenta= {1, 0, 1, 1};
const vec4_t		colorCyan	= {0, 1, 1, 1};
const vec4_t		colorWhite	= {1, 1, 1, 1};
const vec4_t		colorLtGrey	= {0.75, 0.75, 0.75, 1};
const vec4_t		colorMdGrey	= {0.5, 0.5, 0.5, 1};
const vec4_t		colorDkGrey	= {0.25, 0.25, 0.25, 1};

// actually there are 35 colors but we want to use bitmask safely
const vec4_t g_color_table[ 64 ] = {

	{0.0f, 0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f, 1.0f},
	{0.0f, 1.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 0.0f, 1.0f},
	{0.2f, 0.2f, 1.0f, 1.0f}, //{0.0, 0.0, 1.0, 1.0},
	{0.0f, 1.0f, 1.0f, 1.0f},
	{1.0f, 0.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},

	// extended color codes from CPMA/CNQ3:
	{ 1.00000f, 0.50000f, 0.00000f, 1.00000f },	// 8
	{ 0.60000f, 0.60000f, 1.00000f, 1.00000f },	// 9

	// CPMA's alphabet rainbow
	{ 1.00000f, 0.00000f, 0.00000f, 1.00000f },	// a
	{ 1.00000f, 0.26795f, 0.00000f, 1.00000f },	// b
	{ 1.00000f, 0.50000f, 0.00000f, 1.00000f },	// c
	{ 1.00000f, 0.73205f, 0.00000f, 1.00000f },	// d
	{ 1.00000f, 1.00000f, 0.00000f, 1.00000f },	// e
	{ 0.73205f, 1.00000f, 0.00000f, 1.00000f },	// f
	{ 0.50000f, 1.00000f, 0.00000f, 1.00000f },	// g
	{ 0.26795f, 1.00000f, 0.00000f, 1.00000f },	// h
	{ 0.00000f, 1.00000f, 0.00000f, 1.00000f },	// i
	{ 0.00000f, 1.00000f, 0.26795f, 1.00000f },	// j
	{ 0.00000f, 1.00000f, 0.50000f, 1.00000f },	// k
	{ 0.00000f, 1.00000f, 0.73205f, 1.00000f },	// l
	{ 0.00000f, 1.00000f, 1.00000f, 1.00000f },	// m
	{ 0.00000f, 0.73205f, 1.00000f, 1.00000f },	// n
	{ 0.00000f, 0.50000f, 1.00000f, 1.00000f },	// o
	{ 0.00000f, 0.26795f, 1.00000f, 1.00000f },	// p
	{ 0.00000f, 0.00000f, 1.00000f, 1.00000f },	// q
	{ 0.26795f, 0.00000f, 1.00000f, 1.00000f },	// r
	{ 0.50000f, 0.00000f, 1.00000f, 1.00000f },	// s
	{ 0.73205f, 0.00000f, 1.00000f, 1.00000f },	// t
	{ 1.00000f, 0.00000f, 1.00000f, 1.00000f },	// u
	{ 1.00000f, 0.00000f, 0.73205f, 1.00000f },	// v
	{ 1.00000f, 0.00000f, 0.50000f, 1.00000f },	// w
	{ 1.00000f, 0.00000f, 0.26795f, 1.00000f },	// x
	{ 1.0, 1.0, 1.0, 1.0 }, // y, white, duped so all colors can be expressed with this palette
	{ 0.5, 0.5, 0.5, 1.0 }, // z, grey
};


int ColorIndexFromChar( char ccode )
{
	if ( ccode >= '0' && ccode <= '9' ) {
		return ( ccode - '0' );
	}
	else if ( ccode >= 'a' && ccode <= 'z' ) {
		return ( ccode - 'a' + 10 );
	}
	else if ( ccode >= 'A' && ccode <= 'Z' ) {
		return ( ccode - 'A' + 10 );
	}
	else {
		return  ColorIndex( S_COLOR_WHITE );
	}
}

float Q_root(float x)
{
	long        i;								// The integer interpretation of x
	float       x_half = x * 0.5f;
	float       r_sqrt = x;
	const float threehalfs = 1.5F;

	// trick c/c++, bit hack
	i = *(long *)&r_sqrt;					    // oh yes, undefined behaviour, who gives a fuck?
	i = 0x5f375a86 - (i >> 1);				    // weird magic base-16 nums
	r_sqrt = *(float *) &i;

	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 1st Newton iteration
	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 2nd Newton iteration

	return x * r_sqrt; // x * (1/sqrt(x)) := sqrt(x)
}

float Q_rsqrt(float number)
{
#ifdef USING_SSE2
	// does this cpu actually support sse2?
	if (!(CPU_flags & CPU_SSE2)) {
		long x;
    	float x2, y;
		const float threehalfs = 1.5F;

    	x2 = number * 0.5F;
    	x = *(long *)&number;                    // evil floating point bit level hacking
    	x = 0x5f3759df - (x >> 1);               // what the fuck?
    	y = *(float *)&x;
    	y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
  	//	y = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

    	return y;
	}

	float ret;
	_mm_store_ss( &ret, _mm_rsqrt_ss( _mm_load_ss( &number ) ) );
	return ret;
#else
    long x;
    float x2, y;
	const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    x = *(long *)&number;                    // evil floating point bit level hacking
    x = 0x5f3759df - (x >> 1);               // what the fuck?
    y = *(float *)&x;
    y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
//  y = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

    return y;
#endif
}
float disBetweenOBJ(const vec3_t src, const vec3_t tar)
{
	if (src[1] == tar[1]) // horizontal
		return src[0] > tar[0] ? (src[0] - tar[0]) : (tar[0] - src[0]);
	else if (src[0] == tar[0]) // vertical
		return src[1] > tar[1] ? (src[1] - tar[1]) : (tar[1] - src[1]);
	else // diagonal
		return Q_root((pow((src[0] - tar[0]), 2) + pow((src[1] - tar[1]), 2)));
}

vec_t VectorNormalize(vec3_t v)
{
	float ilength, length;

	length = DotProduct(v, v);

	if (length) {
#if 1
		// writing it this way allows g++ to recognize that rsqrt can be used
		ilength = 1/(float)sqrt(length);
#else
		ilength = Q_rsqrt(length);
#endif
		length *= ilength;

		v[0] *= ilength;
		v[1] *= ilength;
	}
	return length;
}
