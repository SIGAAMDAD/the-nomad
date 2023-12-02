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

#ifndef CLAMP
#define CLAMP(a,b,c) MIN(MAX((a),(b)),(c))
#endif

#define MAX_GDR_PATH 64

#if defined(__cplusplus) && !defined(__GNUC__) && !defined(__MINGW64__) && !defined(__MINGW32__)
    #define CallBeforeMain(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
    #pragma section(".CRT$XCU",read)
    #define CallBeforeMain_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
    #ifdef _WIN64
        #define CallBeforeMain(f) CallBeforeMain_(f,"")
    #else
        #define CallBeforeMain(f) CallBeforeMain_(f,"_")
    #endif
#else
    #define CallBeforeMain(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

#ifdef _WIN32
	// dlls
	#pragma comment(lib, "comctl32")
	#pragma comment(lib, "gdi32")
	#pragma comment(lib, "winmm")

	#pragma intrinsic(memcpy)
	#pragma intrinsic(memset)
	#pragma intrinsic(fabs)
	#pragma intrinsic(labs)

	#define DLL_EXT ".dll"
	#define PATH_SEP '\\'
	#define PATH_SEP_FOREIGN '/'
	#define GDR_DECL __cdecl
	#define GDR_NEWLINE "\r\n"
	#define DLL_PREFIX ""

	#ifdef _MSC_VER
		#ifdef _WIN64
			#define MSVC_64
		#else
			#define MSVC_32
		#endif
	#endif
	
	#if defined(_MSC_VER) && _MSVC_VER >= 1400
		#define COMPILER_STRING "msvc"
	#elif defined(__MINGW32__)
		#define COMPILER_STRING "mingw64"
		#define ARCH_STRING "x86"
		#define GDR_LITTLE_ENDIAN
	#elif defined(__MINGW64__)
		#define COMPILER_STRING "mingw32"
		#define ARCH_STRING "x64"
		#define GDR_LITTLE_ENDIAN
	#else
		#error "unsupported windows compiler"
	#endif
	#ifdef _WIN64
		#define OS_STRING "win64 " COMPILER_STRING
	#else
		#define OS_STRING "win32 " COMPILER_STRING
	#endif
	#if defined(_M_IX86)
		#ifndef ARCH_STRING
			#define ARCH_STRING "x86"
		#endif
		#define GDR_LITTLE_ENDIAN
		#undef GDRi386
		#define GDRi386 1
		#ifndef __WORDSIZE
			#define __WORDSIZE 32
		#endif
	#endif
	#if defined(_M_AMD64)
		#ifndef ARCH_STRING
			#define ARCH_STRING "x86_64"
		#endif
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
#elif defined(Q3_VM) || defined(__unix__) // !defined _WIN32
	// common unix platform stuff
	#define DLL_EXT ".so"
	#define PATH_SEP '/'
	#define PATH_SEP_FOREIGN '\\'
	#define GDR_DECL
	#define GDR_NEWLINE "\n"
	#define DLL_PREFIX "./"
	#define POSIX

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

#if defined(GDRx64) || defined(arm64)
#define PLATFORM_64BITS
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
	#define GDR_ALIGN(x) __attribute__((alignment(x)))
	#define GDR_ATTRIBUTE(x) __attribute__(x)
	#define GDR_NOINLINE __attribute__((noinline))

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
	#define GDR_ALIGN(x) __declspec(alignment((x)))
	#define GDR_ATTRIBUTE(x)
	#define GDR_NOINLINE __declspec(noinline)

	#ifdef GDR_DLLCOMPILE
		#define GDR_EXPORT __declspec(dllexport)
	#else
		#define GDR_EXPORT __declspec(dllimport)
	#endif
#elif defined(__LCC__)
	#define GDR_INLINE
	#define GDR_WARN_UNUSED
	#define GDR_NORETURN
	#define GDR_ALIGN(x)
	#define GDR_ATTRIBUTE(x)
	#define GDR_EXPORT
	#define GDR_NOINLINE
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

#define PLATFORM_CLASS
#define PLATFORM_INTERFACE
#define PLATFORM_OVERLOAD

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
#define GO_AWAY_MANGLE extern
#endif

#include "n_pch.h"

#ifdef Q3_VM
#include "../game/bg_lib.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>
#include <time.h>
#include <assert.h>
#ifdef _WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif
#endif

#ifndef Q3_VM
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

int GDR_ATTRIBUTE((format(printf, 3, 4))) GDR_DECL Com_snprintf(char *dest, uint32_t size, const char *format, ...);

#define TICRATE 60

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

// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			3
#define PROP_SPACE_WIDTH		8
#define PROP_HEIGHT				27
#define PROP_SMALL_SIZE_SCALE	0.75

#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			75

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

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
int Com_Split( char *in, char **out, uint64_t outsz, int delim );

typedef int32_t nhandle_t;
typedef int32_t sfxHandle_t;

#ifdef Q3_VM
#ifdef _NOMAD_DEBUG
extern void __assert_failure(const char *expr, const char *file, unsigned line);
#define assert(x) ((int)(x) ? (void )(0) : __assert_failure(#x, __FILE__, __LINE__))
#else
#define assert(x)
#endif
#endif

#define CLOCK_TO_MILLISECONDS(ticks) (((ticks)/(double)CLOCKS_PER_SEC)*1000.0)

#define BIT(x) (1<<(x))

#ifdef __unix__
#define sleepfor(x) usleep((x)*1000)
#elif defined(_WIN32)
#define sleepfor(x) Sleep(x)
#endif

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

typedef uint32_t uvec_t;
typedef uvec_t uvec2_t[2];
typedef uvec_t uvec3_t[3];
typedef uvec_t uvec4_t[4];

typedef int32_t ivec_t;
typedef ivec_t ivec2_t[2];
typedef ivec_t ivec3_t[3];
typedef ivec_t ivec4_t[4];

typedef union {
	float f;
	int i;
	unsigned u;
} floatint_t;

typedef union {
	byte rgba[4];
	uint32_t u32;
} color4ub_t;

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

float Q_rsqrt( float f );		// reciprocal square root

float N_log2f( float f );
float N_exp2f( float f );

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
#define VectorSet4(v,x,y,z,w)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z), v[3]=(w))
#define VectorCopy4(b,a)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

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
float N_fabs( float f );

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

float disBetweenOBJ(const vec3_t src, const vec3_t tar);
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

char *COM_SkipPath (char *pathname);
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

#define NUMVERTEXNORMALS	162
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

//=============================================

float Com_Clamp( float min, float max, float value );

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

//
// key / value info strings
//
const char *Info_ValueForKey( const char *s, const char *key );
void Info_Tokenize( const char *s );
const char *Info_ValueForKeyToken( const char *key );
#define Info_SetValueForKey( buf, key, value ) Info_SetValueForKey_s( (buf), MAX_INFO_STRING, (key), (value) )
qboolean Info_SetValueForKey_s( char *s, uint32_t slen, const char *key, const char *value );
qboolean Info_Validate( const char *s );
qboolean Info_ValidateKeyValue( const char *s );
const char *Info_NextPair( const char *s, char *key, char *value );
size_t Info_RemoveKey( char *s, const char *key );


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

typedef int cvartype_t;

enum
{
    CVT_NONE = 0,
    CVT_INT,
    CVT_STRING,
    CVT_FLOAT,
    CVT_BOOL,
	CVT_FSPATH,

    CVT_MAX
};

typedef enum {
	CVG_VM,
	CVG_ENGINE,
	CVG_RENDERER,
	CVG_ALLOCATOR,
	CVG_SOUND,
	CVG_FILESYSTEM,
    CVG_NONE,

    CVG_MAX
} cvarGroup_t;

// cvar flags
#define	CVAR_SAVE		0x0001	// set to cause it to be saved to default.cfg
					// used for system variables, not for player
					// specific configurations
#define	CVAR_USERINFO		0x0002	// sent to server on connect or change
#define	CVAR_SERVERINFO		0x0004	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		0x0008	// these cvars will be duplicated on all clients
#define	CVAR_INIT			0x0010	// don't allow change from console at all,
					// but can be set from the command line
#define	CVAR_LATCH			0x0020	// will only change when C code next does
					// a Cvar_Get(), so it can't be changed
					// without proper initialization.  modified
					// will be set, even though the value hasn't
					// changed yet
#define	CVAR_ROM			0x0040	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	0x0080	// created by a set command
#define	CVAR_TEMP			0x0100	// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			0x0200	// can not be changed if cheats are disabled
#define CVAR_NORESTART		0x0400	// do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED	0x0800	// cvar was created by a server the client connected to.
#define CVAR_VM_CREATED		0x1000	// cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED		0x2000	// prevent modifying this var from VMs or the server

#define CVAR_NODEFAULT		0x4000	// do not write to config if matching with default value

#define CVAR_PRIVATE		0x8000	// can't be read from VM

#define CVAR_DEV		0x10000 // can be set only in developer mode
#define CVAR_NOTABCOMPLETE	0x20000 // no tab completion in console

#define CVAR_ARCHIVE_ND		(CVAR_SAVE | CVAR_NODEFAULT)

// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED		0x40000000	// Cvar was modified
#define CVAR_NONEXISTENT	0x80000000	// Cvar doesn't exist.

typedef int cvarHandle_t;

#define MAX_CVAR_NAME 64
#define MAX_CVAR_VALUE 256

#define CVAR_INVALID_HANDLE -1

typedef struct cvar_s
{
    char		*name;
	char		*s;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	uint32_t	flags;
	qboolean	modified;			// set each time the cvar is changed
	uint32_t	modificationCount;	// incremented each time the cvar is changed
	float		f;  				// Q_atof( string )
	int32_t		i;      			// atoi( string )
	cvartype_t  type;
	char		*mins;
	char		*maxs;
	char		*description;

	struct cvar_s *next;
	struct cvar_s *prev;
	struct cvar_s *hashNext;
	struct cvar_s *hashPrev;
	uint64_t	hashIndex;
	cvarGroup_t	group;				// to track changes
} cvar_t;

typedef struct {
    char s[MAX_CVAR_VALUE];
    float f;
    int i;
    qboolean b;

    unsigned int modificationCount;
    cvarHandle_t handle;
} vmCvar_t;

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p) ( *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK		'0'
#define S_COLOR_RED			'1'
#define S_COLOR_GREEN		'2'
#define S_COLOR_YELLOW		'3'
#define S_COLOR_BLUE		'4'
#define S_COLOR_CYAN		'5'
#define S_COLOR_MAGENTA		'6'
#define S_COLOR_WHITE		'7'
#define S_COLOR_RESET		'8'

#define COLOR_BLACK		"^0"
#define COLOR_RED		"^1"
#define COLOR_GREEN		"^2"
#define COLOR_YELLOW	"^3"
#define COLOR_BLUE		"^4"
#define COLOR_CYAN		"^5"
#define COLOR_MAGENTA	"^6"
#define COLOR_WHITE		"^7"
#define COLOR_RESET		"^8"

extern const vec4_t	g_color_table[ 64 ];
extern int ColorIndexFromChar( char ccode );

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

void Con_AddText(const char *msg);
void Con_DrawConsole(void);
void Con_Init(void);
void Con_Shutdown(void);

void GDR_DECL Con_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
void GDR_DECL Con_DPrintf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));

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

    DIF_HARDEST = DIF_MINORINCONVENIECE,

	NUMDIFS
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

#define BUTTON_WALKING		1
#define BUTTON_ATTACK		2
#define	BUTTON_ANY			2048			// any key whatsoever

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define PADP(base,align) ((void *)PAD((intptr_t)(base),(align)))

#define LERP( a, b, w ) ( ( a ) * ( 1.0f - ( w ) ) + ( b ) * ( w ) )
#define LUMA( red, green, blue ) ( 0.2126f * ( red ) + 0.7152f * ( green ) + 0.0722f * ( blue ) )

// extra debug stuff
#ifndef Q3_VM
#include "n_debug.h"
#endif

#endif
