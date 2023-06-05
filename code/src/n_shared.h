#ifndef _N_SHARED_
#define _N_SHARED_

#pragma once

#ifdef __LCC__
#ifndef Q3_VM
#define Q3_VM
#endif
#endif

#ifndef Q3_VM

#ifndef _NOMAD_VERSION
#   error a version must be supplied when compiling the engine!
#endif

#define CONSTRUCT(class,name,...) ({class* ptr=(class*)Hunk_Alloc(sizeof(class),name,h_low);new (ptr) class(__VA_ARGS__);ptr;})

#define NOMAD_VERSION _NOMAD_VERSION
#define NOMAD_VERSION_UPDATE _NOMAD_VERSION_UPDATE
#define NOMAD_VERSION_PATCH _NOMAD_VERSION_PATCH
#define VSTR_HELPER(x) #x
#define VSTR(x) VSTR_HELPER(x)
#define NOMAD_VERSION_STRING "glnomad v" VSTR(_NOMAD_VERSION) "." VSTR(_NOMAD_VERSION_UPDATE) "." VSTR(_NOMAD_VERSION_PATCH)

#endif

#if !defined(Q3_VM) && !defined(BFF_COMPILER)
#include "n_pch.h"
#include <EABase/eabase.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/algorithm.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/allocator.h>
#include <EASTL/allocator_malloc.h>
#include <EASTL/core_allocator.h>
#include <EASTL/initializer_list.h>
#include <EASTL/array.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/unordered_map.h>

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))
#endif

#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic error "-Wtype-limits"
#pragma GCC diagnostic error "-Woverflow"

#ifdef _MSVC_VER
#pragma warning(disable : 4996) // posix compatibility
#pragma warning(disable : 4267) // size_t to int conversion, possible loss of data
#pragma warning(disable : 4018) // signed/unsigned mismatch
#endif

#if !defined(__GNUC__) && !defined(__clang__)
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

#define MAX_GDR_PATH 64
#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif

#ifdef __GNUC__
#define GDR_NORETURN __attribute__((noreturn))
#elif defined(_MSVC_VER)
#define GDR_NORETURN __declspec(noreturn)
#else
#define GDR_NORETURN
#endif

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
	#define FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define FUNC_SIG __func__
#else
	#define FUNC_SIG "FUNC_SIG unknown"
#endif
#undef assert

#define NUMSECTORS 4
#define SECTOR_MAX_Y 120
#define SECTOR_MAX_X 120
#define MAP_MAX_Y 240
#define MAP_MAX_X 240

#define arraylen(arr) (sizeof(arr)/sizeof(*arr))


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t mat3_t[3][3];
typedef vec_t mat4_t[4][4];
typedef unsigned char byte;

extern const vec3_t vec3_origin;
extern const vec2_t vec2_origin;

typedef enum { qfalse = 0, qtrue = 1 } qboolean;

#define VectorAdd(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];}
#define VectorSubtract(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];}
#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1])
#define VectorCopy(x,y) {x[0]=y[0];x[1]=y[1];}
void CrossProduct(const vec2_t v1, const vec2_t v2, vec2_t out);

int N_strcmp(const char *str1, const char *str2);
int N_strncmp(const char *str1, const char *str2, size_t count);
int N_strcasecmp(const char *str1, const char *str2);
int N_strncasecmp(const char *str1, const char *str2, size_t n);
int N_atoi(const char *s);
float N_atof(const char *s);
size_t N_strlen(const char *str);
char* N_stradd(char *dst, const char *src);
void N_strcpy(char *dest, const char *src);
void N_strncpy(char *dest, const char *src, size_t count);
void* N_memset(void *dest, int fill, size_t count);
void N_memcpy(void *dest, const void *src, size_t count);
void* N_memchr(void *ptr, int c, size_t count);
int N_memcmp(const void *ptr1, const void *ptr2, size_t count);

// main engine headers
#include "n_platform.h"

int GDR_DECL Com_snprintf(char *dest, uint32_t size, const char *fmt, ...);
const char* GDR_DECL va(const char *fmt, ...);

void GDR_NORETURN N_Error(const char *err, ...);

#ifdef _NOMAD_DEBUG
GDR_INLINE void __nomad_assert_fail(const char* expression, const char* file, const char* func, unsigned line)
{
	N_Error(
		"Assertion '%s' failed (Main Engine):\n"
		"  \\file: %s\n"
		"  \\function: %s\n"
		"  \\line: %u\n\nIf this is an SDL2 error, here is the message string: %s\n",
	expression, file, func, line, SDL_GetError());
}
#define assert(x) (((x)) ? void(0) : __nomad_assert_fail(#x,__FILE__,__func__,__LINE__))
#else
#define assert(x)
#endif
#ifndef BFF_COMPILER
#include "n_console.h"
#include "z_heap.h"
#include "string.hpp"
#include "vector.hpp"
#include "n_common.h"
#endif

#ifdef Q3_VM

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long int int64_t;

// everything is 32-bit on the vm/lcc
typedef int ptrdiff_t;
typedef int intptr_t;
typedef unsigned int uintptr_t;
typedef unsigned int size_t;
typedef signed int ssize_t;

#endif


typedef enum
{
	SPR_PLAYR = 0x00,
	SPR_MERC,
	SPR_WALL,
	SPR_WATER,
	SPR_FLOOR_INSIDE,
	SPR_FLOOR_OUTSIDE,
	SPR_DOOR_STATIC,
	SPR_DOOR_OPEN,
	SPR_DOOR_CLOSE,
	SPR_ROCK,
	SPR_CUSTOM,

	NUMSPRITES
} sprite_t;

typedef enum
{
	D_NORTH,
	D_WEST,
	D_SOUTH,
	D_EAST,

	NUMDIRS,

	D_NULL
} dirtype_t;

// math stuff
float disBetweenOBJ(const vec2_t src, const vec2_t tar);
float Q_rsqrt(float number);
float Q_root(float x);

typedef enum
{
    R_SDL2,
    R_OPENGL,
    R_VULKAN
} renderapi_t;

#define MAX_TICRATE 333
#define MIN_TICRATE 10
#define MAX_VERT_FOV 100
#define MAX_HORZ_FOV 250
#define MSAA_OFF 0
#define MSAA_4X 1
#define MSAA_8X 2
#define MSAA_16X 3

#ifdef Q3_VM
static qboolean N_strtobool(const char *s)
{
	return N_strcmp(s, "true") ? qtrue : qfalse;
}
static const char* N_booltostr(qboolean b)
{
	return b ? "true" : "false";
}
#else
inline qboolean N_strtobool(const char *s)
{
	return N_strcmp(s, "true") ? qtrue : qfalse;
}
inline const char* N_booltostr(qboolean b)
{
	return b ? "true" : "false";
}
#endif

// c++ stuff here
#if !defined(Q3_VM) && !defined(BFF_COMPILER)

extern int myargc;
extern char** myargv;

using json = nlohmann::json;

// engine-only file stuff
int I_GetParm(const char *parm);
size_t N_ReadFile(const char *filepath, void *buffer);
size_t N_LoadFile(const char *filepath, void **buffer);
size_t N_FileSize(const char *filepath);
void N_WriteFile(const char *filepath, const void *data, size_t size);
float disBetweenOBJ(const vec2_t src, const vec2_t tar);
float disBetweenOBJ(const glm::vec2& src, const glm::vec2& tar);
float disBetweenOBJ(const glm::vec3& src, const glm::vec3& tar);

typedef struct coord_s
{
	float y, x;
	inline coord_s() = default;
	inline coord_s(float _y, float _x)
		: y(_y), x(_x)
	{
	}
	inline coord_s(const coord_s &) = default;
	inline coord_s(coord_s &&) = default;
	inline ~coord_s() = default;
	
	inline bool operator==(const coord_s& c) const
	{ return (y == c.y && x == c.x); }
	inline bool operator!=(const coord_s& c) const
	{ return (y != c.y && x != c.x); }
	inline bool operator>(const coord_s& c) const
	{ return (y > c.y && x > c.x); }
	inline bool operator<(const coord_s& c) const
	{ return (y < c.y && x < c.x); }
	inline bool operator>=(const coord_s& c) const
	{ return (y >= c.y && x <= c.x); }
	inline bool operator<=(const coord_s& c) const
	{ return (y <= c.y && x <= c.x); }
	
	inline coord_s& operator++(int) {
		++y;
		++x;
		return *this;
	}
	inline coord_s& operator++(void) {
		y++;
		x++;
		return *this;
	}
	inline coord_s& operator--(int) {
		--y;
		--x;
		return *this;
	}
	inline coord_s& operator--(void) {
		y--;
		x--;
		return *this;
	}
	inline coord_s& operator+=(const coord_s& c) {
		y += c.y;
		x += c.x;
		return *this;
	}
	inline coord_s& operator-=(const coord_s& c) {
		y -= c.y;
		x -= c.x;
		return *this;
	}
	inline coord_s& operator*=(const coord_s& c) {
		y *= c.y;
		x *= c.x;
		return *this;
	}
	
	inline coord_s& operator=(const coord_s& c) {
		y = c.y;
		x = c.x;
		return *this;
	}
	inline coord_s& operator=(const float p) {
		y = p;
		x = p;
		return *this;
	}
} coord_t;

float disBetweenOBJ(const glm::vec3& src, const glm::vec3& tar);
float disBetweenOBJ(const glm::vec2& src, const glm::vec2& tar);
float disBetweenOBJ(const coord_t& src, const coord_t& tar);

constexpr const char* about_str =
"The Nomad has been a project in the making for a long time now, and its been a hard game to,\n"
"develop. The game's idea originally came from fan-fiction after I played Modern Warfare 2's Remastered\n"
"Campaign for the second time around, it used to take place in right about 2020-2080, but then during\n"
"the development I watched and feel in love with Mad Max: Fury Road. I changed up the setting to fit\n"
"more of a desert-planet style of post-apocalypse, but when I was doing this, I thought: \"How can I\n"
"use the already developed content?\", and that right there is exactly how the notorious mercenary guilds\n"
"of Bellatum Terrae were born.\n"
"\nHave fun,\nYour Resident Fiend, Noah Van Til\n\n(IN COLLABORATION WITH GDR GAMES)\n";
constexpr const char* credits_str =
"\n"
"That one slightly aggressive victorian-london-vibey piano piece you'll hear on the ever so occassion:\n    alpecagrenade\n"
"Programming: Noah Van Til (and most of the music as well)\n"
"Concept Artists & Ideas Contributers: Cooper & Tucker Kemnitz\n"
"A Few of the Guns: Ben Pavlovic\n"
"\n";

#define IMGUI_USER_CONFIG "imconfig.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define UPPER_CASE(x) (char)((x) - 32)
#define UPPER_CASE_STR(x) (const char *)((x) - 32)
#define MOUSE_WHEELUP   5
#define MOUSE_WHEELDOWN 6

inline const char* N_ButtonToString(const uint32_t& code)
{
	switch (code) {
	case SDLK_a:
	case SDLK_b:
	case SDLK_c:
	case SDLK_d:
	case SDLK_e:
	case SDLK_f:
	case SDLK_g:
	case SDLK_h:
	case SDLK_i:
	case SDLK_j:
	case SDLK_k:
	case SDLK_l:
	case SDLK_m:
	case SDLK_n:
	case SDLK_o:
	case SDLK_p:
	case SDLK_q:
	case SDLK_r:
	case SDLK_s:
	case SDLK_t:
	case SDLK_u:
	case SDLK_v:
	case SDLK_w:
	case SDLK_x:
	case SDLK_y:
	case SDLK_z:
	case SDLK_0:
	case SDLK_1:
	case SDLK_2:
	case SDLK_3:
	case SDLK_4:
	case SDLK_5:
	case SDLK_6:
	case SDLK_7:
	case SDLK_8:
	case SDLK_9:
	case SDLK_BACKSLASH:
	case SDLK_SLASH:
	case SDLK_PERIOD:
	case SDLK_COMMA:
	case SDLK_SEMICOLON:
	case SDLK_QUOTE:
	case SDLK_MINUS:
	case SDLK_EQUALS:
	case SDLK_LEFTBRACKET:
	case SDLK_RIGHTBRACKET:
	case SDLK_LEFTPAREN:
	case SDLK_RIGHTPAREN:
		return (const char *)&code;
		break;
	case SDL_BUTTON_LEFT: return "Mouse Button Left"; break;
	case SDL_BUTTON_RIGHT: return "Mouse Button Right"; break;
	case SDL_BUTTON_MIDDLE: return "Mouse Button Middle"; break;
	case MOUSE_WHEELDOWN: return "Mouse Wheel Down"; break;
	case MOUSE_WHEELUP: return "Mouse Wheel Up"; break;
	case SDLK_RETURN: return "Enter"; break;
	case SDLK_LCTRL: return "Left Control"; break;
	case SDLK_RCTRL: return "Right Control"; break;
	case SDLK_SPACE: return "Space"; break;
	case SDLK_BACKSPACE: return "Backspace"; break;
	case SDLK_RIGHT: return "Right Arrow"; break;
	case SDLK_UP: return "up Arrow"; break;
	case SDLK_LEFT: return "Left Arrow"; break;
	case SDLK_DOWN: return "Down Arrow"; break;
	case SDLK_TAB: return "Tab"; break;
	case SDLK_BACKQUOTE: return "Grave"; break;
	case SDLK_ESCAPE: return "Escape"; break;
	case SDLK_F1: return "F1"; break;
	case SDLK_F2: return "F2"; break;
	case SDLK_F3: return "F3"; break;
	case SDLK_F4: return "F4"; break;
	case SDLK_F5: return "F5"; break;
	case SDLK_F6: return "F6"; break;
	case SDLK_F7: return "F7"; break;
	case SDLK_F8: return "F8"; break;
	case SDLK_F9: return "F9"; break;
	case SDLK_F10: return "F10"; break;
	case SDLK_F11: return "F11"; break;
	case SDLK_F12: return "F12"; break;
	default:
		Con_Printf("WARNING: unknown SDL_KeyCode given");
		return "Unknown";
		break;
	};
}

template<typename T>
using nomadvector = eastl::vector<T, nomad_allocator<T>>;
template<typename Key, typename T>
using nomad_hashtable = eastl::unordered_map<Key, T, eastl::hash<Key>, eastl::equal_to<Key>, nomad_allocator<T>, false>;
using nomadstring = eastl::basic_string<char, nomad_allocator<char>>;

#endif

#endif
