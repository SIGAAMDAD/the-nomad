#ifndef _N_SHARED_
#define _N_SHARED_

#pragma once

#define GLN_VERSION "GLNomad 1.0 Alpha"
#define WINDOW_TITLE "The Nomad"

/*
=========================================

Platform Specific Preprocessors

=========================================
*/

#define GDRx64  0
#define GDRi386 0
#define arm64   0
#define arm32   0

#define MAX_GDR_PATH 64

#ifdef _WIN32
	#define DLL_EXT ".dll"
	#define PATH_SEP '\\'
	#define PATH_SEP_FOREIGN '/'
	#define GDR_DECL __cdecl
	#define GDR_NEWLINE "\r\n"
	
	#if defined(_MSVC_VER) && _MSVC_VER >= 1400
		#define COMPILER_STRING "msvc"
	#elif defined(__MINGW32__)
		#define COMPILER_STRING "mingw64"
	#elif defined(__MINGW64__)
		#define COMPILER_STRING "mingw32"
	#else
		#error "unsupported windows compiler"
	#endif
	#ifdef _WIN64
		#define OS_STRING "win64 " COMPILER_STRING
	#else
		#define OS_STRING "win32 " COMPILER_STRING
	#endif
	#if defined(_M_IX86)
		#define ARCH_STRING "x86"
		#define GDR_LITTLE_ENDIAN
		#undef GDRi386
		#define GDRi386 1
		#ifndef __WORDSIZE
			#define __WORDSIZE 32
		#endif
	#endif
	#if defined(_M_AMD64)
		#define ARCH_STRING "x86_64"
		#define GDR_LITTLE_ENDIAN
		#undef GDRx64
		#define GDRx64 1
		#ifndef __WORDSIZE
			#define __WORDSIZE 64
		#endif
	#endif
	#if defined(_M_ARM64)
		#define ARCH_STRING "arm64"
		#define GDR_LITTLE_ENDIAN
		#undef arm64
		#define arm64 1
		#ifndef __WORDSIZE
			#define __WORDSIZE 64
		#endif
	#endif
#elif defined(__unix__) // !defined _WIN32
	// common unix platform stuff
	#define DLL_EXT ".so"
	#define PATH_SEP '/'
	#define PATH_SEP_FOREIGN '\\'
	#define GDR_DECL
	#define GDR_NEWLINE "\n"
	#if defined(__i386__)
		#define ARCH_STRING "i386"
		#define GDR_LITTLE_ENDIAN
		#undef GDRi386
		#define GDRi386 1
	#endif
	#if defined(__x86_64__) || defined(__amd64__)
		#define ARCH_STRING "x86-64"
		#define GDR_LITTLE_ENDIAN
		#undef GDRx64
		#define GDRx64 1
	#endif
	#if defined(__arm__)
		#define ARCH_STRING "arm"
		#define GDR_LITTLE_ENDIAN
		#undef arm32
		#define arm32 1
	#endif
	#if defined(__aarch64__)
		#define ARCH_STRING "aarch64"
		#define GDR_LITTLE_ENDIAN
		#undef arm64
		#define arm64 1
	#endif
#else
	#error "WTF are u compiling on?" // seriously
#endif

// linux is defined on android before __ANDROID__
#if defined(__ANDROID__)
	#define OS_STRING "android"
	#error "android isn't yet supported"
#endif
#if defined(__linux__)
	#include <endian.h>
	#define OS_STRING "linux"
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	#include <sys/types.h>
	#include <machine/endian.h>
	#ifdef __FreeBSD__
		#define OS_STRING "freebsd"
	#elif defined(__OpenBSD__)
		#define OS_STRING "openbsd"
	#elif defined(__NetBSD__)
		#define OS_STRING "netbsd"
	#endif
	#if BYTE_ORDER == BIG_ENDIAN
		#define GDR_BIG_ENDIAN
	#else
		#define GDR_LITTLE_ENDIAN
	#endif
#endif

#ifdef __APPLE__
	#define OS_STRING "macos"
	#undef DLL_EXT
	#define DLL_EXT ".dylib"
#endif

#ifdef Q3_VM
	#define OS_STRING "q3vm"
	#define ARCH_STRING "bytecode"
	#define GDR_LITTLE_ENDIAN
	#undef DLL_EXT
	#define DLL_EXT ".qvm"
#endif

#if !defined( OS_STRING )
#error "Operating system not supported"
#endif

#if !defined( ARCH_STRING )
#error "Architecture not supported"
#endif

#ifndef PATH_SEP
#error "PATH_SEP not defined"
#endif

#ifndef PATH_SEP_FOREIGN
#error "PATH_SEP_FOREIGN not defined"
#endif

#ifndef DLL_EXT
#error "DLL_EXT not defined"
#endif

#if defined( GDR_BIG_ENDIAN ) && defined( GDR_LITTLE_ENDIAN )
#error "Endianness defined as both big and little"
#elif defined( GDR_BIG_ENDIAN )

#define CopyLittleShort(dest, src) CopyShortSwap(dest, src)
#define CopyLitteInt(dest, src) CopyIntSwap(dest, src)
#define CopyLittleLong(dest, src) CopyLongSwap(dest, src)
#define LittleShort(x) SDL_SwapLE16(x)
#define LittleInt(x) SDL_SwapLE32(x)
#define LittleLong(x) SDL_SwapLE64(x)
#define LittleFloat(x) SDL_SwapFloatLE(x)
#define BigShort(x) x
#define BigInt(x) x
#define BigLong(x) x
#define BigFloat(x) x

#elif defined( GDR_LITTLE_ENDIAN )

#define CopyLittleShort(dest, src) memcpy(dest, src, 2)
#define CopyLittleInt(dest, src) memcpy(dest, src, 4)
#define CopyLittleLong(dest, src) memcpy(dest, src, 8)
#define LittleShort(x) x
#define LittleInt(x) x
#define LittleLong(x) x
#define LittleFloat(x) x
#define BigShort(x) SDL_SwapBE16(x)
#define BigInt(x) SDL_SwapBE32(x)
#define BigLong(x) SDL_SwapBE64(x)
#define BigFloat(x) SDL_SwapFloatBE(x)

#else
#error "Endianness not defined"
#endif


/*
=========================================

Compiler Macro Abstraction

=========================================
*/
#if defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)
	#define GDR_INLINE __attribute__((always_inline)) inline
	#define GDR_WARN_UNUSED __attribute__((warn_unused_result))
	#define GDR_NORETURN __attribute__((noreturn))
	#define GDR_ALIGN(x) __attribute__((align((x))))
	#define GDR_ATTRIBUTE(x) __attribute__(x)

	#ifdef __GNUC__
		#define GDR_EXPORT __attribute__((visibility("default")))
	#else
		#ifdef GDR_DLLCOMPILE
			#define GDR_EXPORT __declspec(dllexport)
		#else
			#define GDR_EXPORT __declspec(dllimport)
		#endif
	#endif
#elif defined(_MSC_VER)
	#define GDR_INLINE __forceinline
	#define GDR_WARN_UNUSED _Check_return
	#define GDR_NORETURN __declspec(noreturn)
	#define GDR_ALIGN(x) __declspec(align((x)))
	#define GDR_ATTRIBUTE(x)

	#ifdef GDR_DLLCOMPILE
		#define GDR_EXPORT __declspec(dllexport)
	#else
		#define GDR_EXPORT __declspec(dllimport)
	#endif
#endif

// stack based version of strdup
#ifndef strdup_a
	// strdupa is defined in gcc
	#ifdef strdupa
		#define strdup_a strdupa
	#else
		#define strdup_a(str) (char*)memcpy(memset(alloca(strlen((str))+1),0,strlen(str)+1),str,strlen(str))
	#endif
#endif

// make sure Q3_VM is defined
#ifdef __LCC__
	#ifndef Q3_VM
		#define Q3_VM
	#endif
#endif

#ifndef _NOMAD_VERSION
#   error a version must be supplied when compiling the engine or a mod
#endif

#ifdef __GNUG__
#pragma GCC diagnostic ignored "-Wold-style-cast" // c style stuff, its more readable without the syntax sugar
#pragma GCC diagnostic ignored "-Wclass-memaccess" // non-trivial copy instead of memcpy
#pragma GCC diagnostic ignored "-Wcast-function-type" // incompatible function pointer types
#pragma GCC diagnostic ignored "-Wunused-macros"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wnoexcept"

#pragma GCC diagnostic error "-Walloca-larger-than=1048576" // prevent stack overflows (allocating more than a mb on the stack)
#elif defined(__clang__) && defined(__cplusplus)
#pragma clang diagnostic ignored "-Wold-style-cast" // c style stuff, its more readable without the syntax sugar
#pragma clang diagnostic ignored "-Wclass-memaccess" // non-trivial copy instead of memcpy
#pragma clang diagnostic ignored "-Wcast-function-type" // incompatible function pointer types
#pragma clang diagnostic ignored "-Wunused-macros"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wnoexcept"

#pragma clang diagnostic error "-Walloca-larger-than=1048576" // prevent stack overflows (allocating more than a mb on the stack)
#endif

#define NOMAD_VERSION_FULL \
			(uint32_t)(_NOMAD_VERSION * 10000 \
						+ _NOMAD_VERSION_UPDATE * 100 \
						+ _NOMAD_VERSION_PATCH)
#define NOMAD_VERSION _NOMAD_VERSION
#define NOMAD_VERSION_UPDATE _NOMAD_VERSION_UPDATE
#define NOMAD_VERSION_PATCH _NOMAD_VERSION_PATCH
#define VSTR_HELPER(x) #x
#define VSTR(x) VSTR_HELPER(x)
#define NOMAD_VERSION_STRING "glnomad v" VSTR(_NOMAD_VERSION) "." VSTR(_NOMAD_VERSION_UPDATE) "." VSTR(_NOMAD_VERSION_PATCH)

// disable name-mangling
#ifdef __cplusplus
#define GO_AWAY_MANGLE extern "C"
#else
#define GO_AWAY_MANGLE
#endif

#include "n_pch.h"

#ifdef Q3_VM
#include "../game/bg_lib.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#endif

int GDR_ATTRIBUTE((format(printf, 3, 4))) GDR_DECL Com_snprintf(char *dest, uint32_t size, const char *format, ...);

#ifndef M_LN2
#define M_LN2          0.69314718055994530942  /* log_e 2 */
#endif
#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
#ifdef __cplusplus
typedef uint32_t qboolean;
#define qtrue 1
#define qfalse 0
#else
typedef enum { qfalse = 0, qtrue = 1 } qboolean;
#endif
#endif

const char *Com_SkipTokens( const char *s, uint64_t numTokens, const char *sep );
const char *Com_SkipCharset( const char *s, const char *sep );
int N_isprint(int c);
int N_islower(int c);
int N_isupper(int c);
int N_isalpha(int c);
qboolean N_isintegral(float f);
qboolean N_isanumber(const char *s);
int N_strcmp(const char *str1, const char *str2);
int N_strncmp(const char *str1, const char *str2, size_t count);
int N_stricmp(const char *str1, const char *str2);
int N_stricmpn(const char *str1, const char *str2, size_t n);
char *N_strlwr(char *s1);
char *N_strupr(char *s1);
void N_strcat(char *dest, size_t size, const char *src);
const char *N_stristr(const char *s, const char *find);
float N_fmaxf(float a, float b);
int N_atoi(const char *s);
float N_atof(const char *s);
size_t N_strlen(const char *str);
qboolean N_streq(const char *str1, const char *str2);
qboolean N_strneq(const char *str1, const char *str2, size_t n);
char* N_stradd(char *dst, const char *src);
void N_strcpy(char *dest, const char *src);
void N_strncpy(char *dest, const char *src, size_t count);
void N_strncpyz(char *dest, const char *src, size_t count);
void* N_memset(void *dest, int fill, size_t count);
void N_memcpy(void *dest, const void *src, size_t count);
void* N_memchr(void *ptr, int c, size_t count);
int N_memcmp(const void *ptr1, const void *ptr2, size_t count);
int N_replace(const char *str1, const char *str2, char *src, size_t max_len);

typedef int32_t nhandle_t;
typedef int32_t sfxHandle_t;

#define CLOCK_TO_MILLISECONDS(ticks) (((ticks)/(double)CLOCKS_PER_SEC)*1000.0)

#define BIT(x) (1<<(x))

#ifdef __unix__
#define sleepfor(x) usleep((x)*1000)
#elif defined(_WIN32)
#define sleepfor(x) Sleep(x)
#endif

#include "../allocator/z_heap.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t mat3_t[3][3];
typedef vec_t mat4_t[4][4];
typedef unsigned char byte;

typedef union {
	float f;
	int i;
	unsigned u;
} floatint_t;

extern const mat4_t mat4_identity;
extern const vec3_t vec3_origin;
extern const vec2_t vec2_origin;

extern	const vec4_t		colorBlack;
extern	const vec4_t		colorRed;
extern	const vec4_t		colorGreen;
extern	const vec4_t		colorBlue;
extern	const vec4_t		colorYellow;
extern	const vec4_t		colorMagenta;
extern	const vec4_t		colorCyan;
extern	const vec4_t		colorWhite;
extern	const vec4_t		colorLtGrey;
extern	const vec4_t		colorMdGrey;
extern	const vec4_t		colorDkGrey;

extern const byte locase[256];

#ifdef Q3_VM
	typedef int intptr_t;
#else
	#if defined (_MSC_VER) && !defined(__clang__)
		typedef __int64 int64_t;
		typedef __int32 int32_t;
		typedef __int16 int16_t;
		typedef __int8 int8_t;
		typedef unsigned __int64 uint64_t;
		typedef unsigned __int32 uint32_t;
		typedef unsigned __int16 uint16_t;
		typedef unsigned __int8 uint8_t;
	#else
		#include <stdint.h>
	#endif

	#ifdef _WIN32
		// vsnprintf is ISO/IEC 9899:1999
		// abstracting this to make it portable
		int N_vsnprintf( char *str, size_t size, const char *format, va_list ap );
	#else
		#define N_vsnprintf vsnprintf
	#endif
#endif

#if defined (_WIN32)
#if !defined(_MSC_VER)
// use GCC/Clang functions
#define Q_setjmp __builtin_setjmp
#define Q_longjmp __builtin_longjmp
#elif GDRx64 && (_MSC_VER >= 1910)
// use custom setjmp()/longjmp() implementations
#define Q_setjmp Q_setjmp_c
#define Q_longjmp Q_longjmp_c
int Q_setjmp_c(void *);
int Q_longjmp_c(void *, int);
#else // !GDRx64 || MSVC<2017
#define Q_setjmp setjmp
#define Q_longjmp longjmp
#endif
#else // !_WIN32
#define Q_setjmp setjmp
#define Q_longjmp longjmp
#endif

#ifdef ERR_FATAL
	#undef ERR_FATAL // this is defined in malloc.h
#endif

// error codes
typedef enum {
	ERR_FATAL,		// exit the entire game with a popup window
	ERR_DROP,		// print to console and go to title screen
} errorCode_t;
void GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *fmt, ...);

#define arraylen(arr) (sizeof((arr))/sizeof(*(arr)))
#define zeroinit(x,size) memset((x),0,(size))

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

float Q_fabs( float f );
float Q_rsqrt( float f );		// reciprocal square root

float Q_log2f( float f );
float Q_exp2f( float f );

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

signed char ClampChar( int i );
signed char ClampCharMove( int i );
signed short ClampShort( int i );

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#ifndef SGN
#define SGN(x) (((x) >= 0) ? !!(x) : -1)
#endif

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

#if	1

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(b,a)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))

#define DotProduct4(a,b)		((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2] + (a)[3]*(b)[3])
#define VectorScale4(a,b,c)		((c)[0]=(a)[0]*(b),(c)[1]=(a)[1]*(b),(c)[2]=(a)[2]*(b),(c)[3]=(a)[3]*(b))
#define VectorCopy2(dst,src)	((dst)[0]=(src)[0],(dst)[1]=(src)[1])
#define VectorSet2(dst,a,b)		((dst)[0]=(a),(dst)[1]=(b))

#else

#define DotProduct(x,y)			_DotProduct(x,y)
#define VectorSubtract(a,b,c)	_VectorSubtract(a,b,c)
#define VectorAdd(a,b,c)		_VectorAdd(a,b,c)
#define VectorCopy(b,a)			_VectorCopy(a,b)
#define	VectorScale(v, s, o)	_VectorScale(v,s,o)
#define	VectorMA(v, s, b, o)	_VectorMA(v,s,b,o)

#endif

#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_INT			0x7fffffff
#define	MIN_INT			(-MAX_INT-1)

#define	MAX_UINT			((unsigned)(~0))

#ifdef Q3_VM
#ifdef VectorCopy
#undef VectorCopy
// this is a little hack to get more efficient copies in our interpreter
typedef struct {
	float	v[3];
} vec3struct_t;
#define VectorCopy(a,b)	(*(vec3struct_t *)b=*(vec3struct_t *)a)
#endif
#endif

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Set(v,x,y,z,w)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z), v[3]=(w))
#define Vector4Copy(b,a)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define Byte4Copy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define QuatCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define	SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
// just in case you don't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

#if !defined( Q3_VM ) || ( defined( Q3_VM ) && defined( __Q3_VM_MATH ) )
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

#else

int VectorCompare( const vec3_t v1, const vec3_t v2 );
vec_t VectorLength( const vec3_t v );
vec_t VectorLengthSquared( const vec3_t v );
vec_t Distance( const vec3_t p1, const vec3_t p2 );
vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );
void VectorNormalizeFast( vec3_t v );
void VectorInverse( vec3_t v );
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );

#endif

vec_t VectorNormalize (vec3_t v);		// returns vector length
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( const vec3_t in, const vec3_t matrix[3], vec3_t out );
int N_log2(int val);

float N_acos(float c);

int N_rand( int *seed );
float N_random( int *seed );
float N_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

void vectoangles( const vec3_t value1, vec3_t angles);
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );

void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

//void SetPlaneSignbits( struct cplane_s *out );
//int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

qboolean BoundsIntersect(const vec3_t mins, const vec3_t maxs,
		const vec3_t mins2, const vec3_t maxs2);
qboolean BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin, vec_t radius);
qboolean BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin);

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

//int	PlaneTypeForNormal (vec3_t normal);

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );
int N_isnan( float x );

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

//=============================================

float Com_Clamp( float min, float max, float value );

#include "n_console.h"
#include "n_common.h"

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

int I_GetParm(const char *parm);

typedef enum
{
    DIF_NOOB,
    DIF_RECRUIT,
    DIF_MERC,
    DIF_NOMAD,
    DIF_BLACKDEATH,
    DIF_MINORINCONVENIECE,

    DIF_HARDEST = DIF_MINORINCONVENIECE
} gamedif_t;

typedef enum
{
	D_NORTH,
	D_WEST,
	D_SOUTH,
	D_EAST,

	NUMDIRS,

	D_NULL
} dirtype_t;

typedef enum
{
    R_SDL2,
    R_OPENGL,
    R_VULKAN
} renderapi_t;

#ifdef __cplusplus
template<typename type, typename alignment>
inline type* PADP(type *base, alignment align)
{
	return (type *)((void *)PAD((intptr_t)base, align));
}
#else
#define PADP(base,align) ((void *)PAD((intptr_t)(base),(align)))
#endif

#endif
