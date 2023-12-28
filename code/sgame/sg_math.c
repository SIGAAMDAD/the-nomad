#include "sg_local.h"


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

int		Q_rand( int *seed ) {
	*seed = (69069 * *seed + 1);
	return *seed;
}

float	Q_random( int *seed ) {
	return ( Q_rand( seed ) & 0xffff ) / (float)0x10000;
}

float	Q_crandom( int *seed ) {
	return 2.0 * ( Q_random( seed ) - 0.5 );
}

//=======================================================

signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

signed char ClampCharMove( int i ) {
	if ( i < -127 ) {
		return -127;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return i;
}

unsigned ColorBytes3( float r, float g, float b ) {
    typedef struct {
        byte r, g, b;
    } u32color_t;
    u32color_t i;

    memset( &i, 0, sizeof(i) );

#if 1
    i.r = r * 255;
    i.g = g * 255;
    i.b = b * 255;
#else
	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
#endif

    return *(unsigned *)&i;
}

unsigned ColorBytes4( float r, float g, float b, float a ) {
    typedef struct {
        byte r, g, b, a;
    } u32color_t;
    u32color_t i;

#if 1
    i.r = r * 255;
    i.g = g * 255;
    i.b = b * 255;
    i.a = a * 255;
#else
	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;
#endif

    return *(unsigned *)&i;
}

float NormalizeColor( const vec3_t *in, vec3_t *out ) {
	float	max;
	
	max = in->x;

	if ( in->y > max ) {
		max = in->y;
	}
	if ( in->z > max ) {
		max = in->z;
	}

	if ( !max ) {
		memset( out, 0, sizeof(*out) );
	} else {
		out->x = in->x / max;
		out->y = in->y / max;
		out->z = in->z / max;
	}
	return max;
}

float Q_rsqrt(float number)
{
#ifdef USING_SSE2
	// does this cpu actually support sse2?
	if ( !( CPU_flags & CPU_SSE2 ) ) {
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
float disBetweenOBJ(const vec3_t *src, const vec3_t *tar)
{
	if ( src->y == tar->y ) { // horizontal
		return src->x > tar->x ? ( src->x - tar->x ) : ( tar->x - src->x );
    } else if ( src->x == tar->x ) { // vertical
		return src->y > tar->y ? ( src->y - tar->y ) : (tar->y - src->y );
    } else { // diagonal
		return sqrtf( ( pow( ( src->x - tar->x ), 2 ) + pow( ( src->y - tar->y ), 2 ) ) );
    }
}

#if !defined(__Q3_VM_MATH)
void CrossProduct(const vec3_t *v1, const vec3_t *v2, vec3_t *cross)
{
	cross->x = v1->y*v2->z - v1->z*v2->y;
	cross->y = v1->z*v2->x - v1->x*v2->z;
	cross->z = v1->x*v2->y - v1->y*v2->x;
}

vec_t VectorLength(const vec3_t *v) {
	return (vec_t)sqrtf ( v->x*v->x + v->y*v->y + v->z*v->z );
}
#endif

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const vec3_t *mins, const vec3_t *maxs ) {
	vec3_t	corner;
	float	a, b;

    a = fabs( mins->x );
    b = fabs( maxs->x );
    corner.x = a > b ? a : b;

    a = fabs( mins->y );
    b = fabs( maxs->y );
    corner.y = a > b ? a : b;

    a = fabs( mins->z );
    b = fabs( maxs->z );
    corner.z = a > b ? a : b;

	return VectorLength( &corner );
}

int VectorCompare( const vec3_t *a, const vec3_t *b ) {
	if ( a->x != b->x || a->y != b->y || a->z != b->z ) {
		return 0;
	}
	return 1;
}

void ClearBounds( vec3_t *mins, vec3_t *maxs ) {
	mins->x = mins->y = mins->z = 99999;
	maxs->x = maxs->y = maxs->z = -99999;
}

void AddPointToBounds( const vec3_t *v, vec3_t *mins, vec3_t *maxs ) {
	if ( v->x < mins->x ) {
		mins->x = v->x;
	}
	if ( v->x > maxs->x ) {
		maxs->x = v->x;
	}

	if ( v->y < mins->y ) {
		mins->y = v->y;
	}
	if ( v->y > maxs->y ) {
		maxs->y = v->y;
	}

	if ( v->z < mins->z ) {
		mins->z = v->z;
	}
	if ( v->z > maxs->z ) {
		maxs->z = v->z;
	}
}

qboolean BoundsIntersect( const bbox_t *a, const bbox_t *b )
{
    if ( a->maxs.x < b->mins.x ||
        a->maxs.y < b->mins.y ||
        a->maxs.z < b->mins.z ||
        a->mins.x > b->maxs.x ||
        a->mins.y > b->maxs.y || 
        a->mins.z > b->maxs.z )
    {
        return qfalse;
    }
	return qtrue;
}


qboolean BoundsIntersectSphere( const bbox_t *box,
		const vec3_t *origin, vec_t radius )
{
	if ( origin->x - radius > box->maxs.x ||
		origin->x + radius < box->mins.x ||
		origin->y - radius > box->maxs.y ||
		origin->y + radius < box->mins.y ||
		origin->z - radius > box->maxs.z ||
		origin->z + radius < box->mins.z)
	{
		return qfalse;
	}

	return qtrue;
}

qboolean BoundsIntersectPoint( const bbox_t *box,
		const vec3_t *origin )
{
	if ( origin->x > box->maxs.x ||
		origin->x < box->mins.x ||
		origin->y > box->maxs.y ||
		origin->y < box->mins.y ||
		origin->z > box->maxs.z ||
		origin->z < box->mins.z)
	{
		return qfalse;
	}

	return qtrue;
}

float vec3_get( vec3_t *v, int i ) {
	switch ( i ) {
	case 0: return v->x;
	case 1: return v->y;
	case 2: return v->z;
	default:
		trap_Error( "vec3_get: bad index" );
		break;
	};
	return 0;
}

vec_t VectorNormalize( vec3_t *v ) {
	// NOTE: TTimo - Apple G4 altivec source uses double?
	float	length, ilength;

	length = v->x*v->x + v->y*v->y + v->z*v->z;

	if ( length ) {
		/* writing it this way allows gcc to recognize that rsqrt can be used */
		ilength = 1/(float)sqrt (length);
		/* sqrt(length) = length * (1 / sqrt(length)) */
		length *= ilength;
		v->x *= ilength;
		v->y *= ilength;
		v->z *= ilength;
	}
		
	return length;
}

vec_t VectorNormalize2( const vec3_t *v, vec3_t *out) {
	float	length, ilength;

	length = v->x*v->x + v->y*v->y + v->z*v->z;

	if (length) {
		/* writing it this way allows gcc to recognize that rsqrt can be used */
		ilength = 1/(float)sqrt (length);
		/* sqrt(length) = length * (1 / sqrt(length)) */
		length *= ilength;
		out->x = v->x*ilength;
		out->y = v->y*ilength;
		out->z = v->z*ilength;
	} else {
		memset( out, 0, sizeof(*out) );
	}
		
	return length;

}

void _VectorMA( const vec3_t *veca, float scale, const vec3_t *vecb, vec3_t *vecc) {
	vecc->x = veca->x + scale*vecb->x;
	vecc->y = veca->y + scale*vecb->y;
	vecc->z = veca->z + scale*vecb->z;
}


vec_t _DotProduct( const vec3_t *v1, const vec3_t *v2 ) {
	return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

void _VectorSubtract( const vec3_t *veca, const vec3_t *vecb, vec3_t *out ) {
	out->x = veca->x-vecb->x;
	out->y = veca->y-vecb->y;
	out->z = veca->z-vecb->z;
}

void _VectorAdd( const vec3_t *veca, const vec3_t *vecb, vec3_t *out ) {
	out->x = veca->x+vecb->x;
	out->y = veca->y+vecb->y;
	out->z = veca->z+vecb->z;
}

void _VectorCopy( const vec3_t *in, vec3_t *out ) {
	out->x = in->x;
	out->y = in->y;
	out->z = in->z;
}

void _VectorScale( const vec3_t *in, vec_t scale, vec3_t *out ) {
	out->x = in->x*scale;
	out->y = in->y*scale;
	out->z = in->z*scale;
}

void Vector4Scale( const vec4_t *in, vec_t scale, vec4_t *out ) {
	out->r = in->r*scale;
	out->g = in->g*scale;
	out->b = in->b*scale;
	out->a = in->a*scale;
}


int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val>>=1 ) != 0 ) {
		answer++;
	}
	return answer;
}

/*
================
Q_isnan

Don't pass doubles to this
================
*/
int N_isnan( float x )
{
	floatint_t fi;

	fi.f = x;
	fi.u &= 0x7FFFFFFF;
	fi.u = 0x7F800000 - fi.u;

	return (int)( fi.u >> 31 );
}
//------------------------------------------------------------------------


/*
================
Q_isfinite
================
*/
static int N_isfinite( float f )
{
	floatint_t fi;
	fi.f = f;

	if ( fi.u == 0xFF800000 || fi.u == 0x7F800000 )
		return 0; // -INF or +INF

	fi.u = 0x7F800000 - (fi.u & 0x7FFFFFFF);
	if ( (int)( fi.u >> 31 ) )
		return 0; // -NAN or +NAN

	return 1;
}


/*
================
Q_atof
================
*/
float N_atof( const char *str )
{
	float f;

	f = atof( str );

	// modern C11-like implementations of atof() may return INF or NAN
	// which breaks all FP code where such values getting passed
	// and effectively corrupts range checks for cvars as well
	if ( !N_isfinite( f ) )
		return 0.0f;

	return f;
}


float N_fabs( float f ) {
	floatint_t fi;
	fi.f = f;
	fi.i &= 0x7FFFFFFF;
	return fi.f;
}


/*
================
Q_log2f
================
*/
float N_log2f( float f )
{
	const float v = logf( f );
	return v / M_LN2;
}


/*
================
Q_exp2f
================
*/
float N_exp2f( float f )
{
	return powf( 2.0f, f );
}
