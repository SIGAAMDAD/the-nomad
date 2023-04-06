#ifndef _N_SHARED_
#define _N_SHARED_

#pragma once

#ifndef M_PI
#define M_PI
#endif

#define RAD2DEG(x)
#define DEG2RAD(x)

#ifndef _NOMAD_VERSION
#   error There must be a _NOMAD_VERSION pre-defined!
#endif

#include "n_pch.h"
#include "nomaddef.h"

#ifdef NULL
#undef NULL
#endif

#define NULL 0

#define NOMAD_VERSION _NOMAD_VERSION
#define NOMAD_VERSION_UPDATE _NOMAD_VERSION_UPDATE
#define NOMAD_VERSION_PATCH _NOMAD_VERSION_PATCH

typedef enum : uint16_t
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

typedef int_fast32_t point_t;
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef uint_fast8_t byte;
typedef byte color_t[4];

enum : byte
{
	D_NORTH,
	D_WEST,
	D_SOUTH,
	D_EAST,

	NUMDIR,

	D_NULL
};

extern bool sdl_on;

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename... Args>
constexpr inline Scope<T> make_scope(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename... Args>
constexpr inline Ref<T> make_ref(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

class Log
{
public:
	static void Init();
	static std::shared_ptr<spdlog::logger>& GetLogger() { return m_Instance; }
private:
	static std::shared_ptr<spdlog::logger> m_Instance;
};

class Console
{
public:
	inline void ConPrintf(const char* fmt, ...)
	{
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(stdout, fmt, argptr);
		va_end(argptr);
		fprintf(stdout, "\n");
	}
	inline void ConError(const char* fmt, ...)
	{
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(stderr, fmt, argptr);
		va_end(argptr);
		fprintf(stderr, "\n");
	}
	inline void ConFlush()
	{
		fflush(stdout);
		fflush(stderr);
	}
};

extern Console con;

void N_Error(const char *err, ...);

typedef struct non_atomic_coord_s
{
	int32_t y, x;
	inline non_atomic_coord_s() = default;
	inline non_atomic_coord_s(int_fast32_t _y, int_fast32_t _x)
		: y(_y), x(_x)
	{ }
} non_atomic_coord_t;

typedef struct coord_s
{
	point_t y, x;
	inline coord_s() = default;
	inline coord_s(int_fast32_t _y, int_fast32_t _x)
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
	inline coord_s& operator=(const int_fast32_t p) {
		y = p;
		x = p;
		return *this;
	}
} coord_t;

typedef struct area_s
{
	coord_t tl, tr;
	coord_t bl, br;
	inline area_s()
		: tl(0, 0), tr(0, 0), bl(0, 0), br(0, 0)
	{
	}
	inline area_s(const area_s &) = default;
	inline area_s(area_s &&) = default;
	inline ~area_s() = default;

	inline bool inArea(const coord_s& c) const
	{ return (c.y >= tl.y && c.y <= br.y) && (c.x >= tl.x && c.x <= br.x); }
} area_t;

typedef struct dim_s
{
	point_t height, width;
	inline dim_s(const dim_s &) = default;
	inline dim_s(dim_s &&) = default;
	inline dim_s() = default;
	inline ~dim_s() = default;
	inline dim_s(int_fast32_t _height, int_fast32_t _width)
	{
		height =_height;
		width = _width;
	}
} dim_t;

#ifdef assert
#undef assert
#endif

#ifdef _NOMAD_DEBUG
inline void __nomad_assert_fail(const char* expression, const char* file, const char* func, unsigned line)
{
	N_Error(
		"Assertion '%s' failed:\n"
		"  \\file: %s\n"
		"  \\function: %s\n"
		"  \\line: %u\n\nIf this is an SDL2 error, here is the message string: %s\n",
	expression, file, func, line, SDL_GetError());
}
#define assert(x) (((bool)(x)) ? void(0) : __nomad_assert_fail(#x,__FILE__,__func__,__LINE__))
#else
#define assert(x)
#endif

constexpr float threehalfs = 1.5f;

template<typename T>
inline float Q_root(T x)
{
	int64_t     i;								// The integer interpretation of x
	float       x_half = x * 0.5f;
	float       r_sqrt = x;

	// trick c/c++, bit hack
	i = *(int64_t *)&r_sqrt;					    // oh yes, undefined behaviour, who gives a fuck?
	i = 0x5f375a86 - (i >> 1);				            // weird magic base-16 nums
	r_sqrt = *(float *) &i;

	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 1st Newton iteration
	r_sqrt = r_sqrt * (threehalfs - (x_half * r_sqrt * r_sqrt)); // 2nd Newton iteration

	return x * r_sqrt; // x * (1/sqrt(x)) := sqrt(x)
}

inline int32_t disBetweenOBJ(const coord_t& src, const coord_t& tar)
{
	if (src.y == tar.y) // horizontal
		return src.x > tar.x ? (src.x - tar.x) : (tar.x - src.x);
	else if (src.x == tar.x) // vertical
		return src.y > tar.y ? (src.y - tar.y) : (tar.y - src.y);
	else // diagonal
		return Q_root((pow((src.x - tar.x), 2) + pow((src.y - tar.y), 2)));
}

template<typename T>
inline T abs(T x) { return x ? -x : x; }

inline bool N_strcmp(const char* str1, const char* str2)
{
	assert(str1 && str2);
	const char *__restrict it1 = str1;
	const char *__restrict it2 = str2;
	while (*it1 && *it2) {
		if (*it1++ != *it2++) return false;
	}
	return true;
}
inline bool N_strncmp(const char* str1, const char* str2, uint_fast64_t count)
{
	assert(str1 && str2 && count > 0);
	const char* it1 = str1;
	const char* it2 = str2;
	while (*it1 && *it2 && --count) {
		if (*it1++ != *it2++) return false;
	}
	return true;
}
inline char *N_strcpy(char *dest, const char* src)
{
	assert(src);
	char *to = dest;
	const char* from = src;
	while (*from)
		*to++ = *from++;
	return dest;
}
inline char *N_strncpy(char *dest, const char* src, uint_fast64_t count)
{
	assert(src && count > 0);
	char *to = dest;
	const char *from = src;
	while (*dest && --count)
		*to++ = *from++;
	return dest;
}
inline size_t N_strlen(const char* str)
{
	assert(str);
	size_t len = 0;
	const char *it = str;
	while (*it++)
		++len;
	return len;
}
inline void *N_memcpy(void *dest, const void *src, uint_fast64_t count)
{
	assert(dest);
	assert(src);
	unsigned char *to = (unsigned char *)dest;
	const unsigned char *from = (const unsigned char *)src;
	while (--count)
		*to++ = *from++;
	return dest;
}
inline bool N_memcmp(const void *ptr1, const void *ptr2, uint_fast64_t count)
{
	assert(ptr1);
	assert(ptr2);
	const unsigned char *it1 = (const unsigned char *)ptr1;
	const unsigned char *it2 = (const unsigned char *)ptr2;
	while (--count) {
		if (*it1++ != *it2++) return false;
	}
	return true;
}
inline void *N_memmove(void *dest, const void *src, uint_fast64_t count)
{
	assert(dest);
	assert(src);
#ifdef _NOMAD_DEBUG
	assert(count > 0);
#else
	if (count < 1) {
		LOG_WARN("count given to N_memmove was less than 1");
		return NULL;
	}
#endif

	unsigned char *to = (unsigned char *)dest;
	const unsigned char *from = (const unsigned char *)src;
	if (to < from)
		while (--count)
			*to++ = *from++;
	else {
		const unsigned char *lastsrc = from + (count - 1);
		unsigned char *lastdest = to + (count - 1);
		while (--count)
			*lastdest++ = *lastsrc++;
	}
	return dest;
}
inline void *N_memset(void *dest, int c, uint_fast64_t count)
{
	assert(dest);
	unsigned char *it = (unsigned char *)dest;
	while (--count)
		*it++ = c;
	return dest;
}
inline const char* N_booltostr(bool b)
{ return b == true ? "true" : "false"; }
inline const char* N_booltostr2(bool b)
{ return b == true ? "yes" : "no"; }
inline bool N_strtobool(const std::string& str)
{ return str == "true" ? true : false; }
inline bool N_strtobool(const char* str)
{ return N_strncmp(str, "true", N_strlen(str)) ? true : false; }
inline bool N_strtobool2(const std::string& str)
{ return str == "yes" ? true : false; }
inline bool N_strtobool2(const char* str)
{ return N_strncmp(str, "yes", N_strlen(str)) ? true : false; }

#define arraylen(arr) (size_t)(sizeof(arr)/sizeof(arr[0]))

#ifdef _MSVC_VER
#define DEBUG_BREAK() __debugbreak()
#elif defined(_GNUC_)
#define DEBUG_BREAK() __builtin_trap()
#endif

#define cvector_clib_free Z_Free
#define cvector_clib_malloc Z_Malloc
#define cvector_clib_realloc Z_Realloc
#define cvector_clib_calloc Z_Calloc
#define CVECTOR_LOGARITHMIC_GROWTH
#include "cvector.h"

#define LOG_INFO(...)  ::spdlog::info(__VA_ARGS__)
#define LOG_WARN(...)  ::spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) ::spdlog::error(__VA_ARGS__)
#ifdef _NOMAD_DEBUG
#define LOG_TRACE(...) ::spdlog::trace(__VA_ARGS__)
#define LOG_DEBUG(...) ::spdlog::debug(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#define LOG_TRACE(...)
#endif

using json = nlohmann::json;

#define SCREEN_HEIGHT scf::renderer::height
#define SCREEN_WIDTH  scf::renderer::width
#define SCREEN_RES    (scf::renderer::height * scf::renderer::width)

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

class thread
{
private:
	pthread_t id;
	void *(*work)(void *);
	void *args;
	bool working;
public:
	inline thread(void *(*_work)(void *), void *_args, bool launch = false)
		: work(_work), args(_args)
	{
		assert(_work);
		create(_args);
	}
	inline thread() = default;
	inline ~thread()
	{
		if (working)
			join();
	}
	inline void create(void *_args = NULL)
	{
		if (working) {
			LOG_WARN("attempted to create a new thread when it is already working");
			return;
		}
		pthread_create(&id, NULL, work, _args);
		LOG_INFO("launching new thread with id %lu", id);
		working = true;
	}
	inline void join(void *_args = NULL)
	{
		if (!working) {
			LOG_WARN("attempted to join thread worker when it isn't working");
			return;
		}
		LOG_INFO("joining worker thread with id %lu back to calling thread of id %lu", id, pthread_self());
		pthread_join(id, &_args);
	}
	thread(const thread &) = delete;
	inline thread(thread &&) = default;
};

class Profiler
{
private:
//    std::chrono::time_point<std::chrono::system_clock> start;
	float *timer;
	clock_t start;
public:
	Profiler(float* _timer)
	{
		assert(_timer);
		timer = _timer;
		start = clock();
//		start = std::chrono::system_clock::now();
	}
	~Profiler()
	{
		clock_t end = clock();
		*timer = (float)(end - start) / (float)CLOCKS_PER_SEC;
  //      std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
//		*timer = (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
};

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

#ifdef _NOMAD_DEBUG
#define IMGUI_BEGIN(name)            ::ImGui::Begin(name)
#define IMGUI_BEGIN_BOOL(name, bool) ::ImGui::Begin(name, bool)
#define IMGUI_TEXT(...)              ::ImGui::Text(__VA_ARGS__)
#define IMGUI_SAMELINE(offset)       ::ImGui::SameLine(offset)
#define IMGUI_END()                  ::ImGui::End()
#else
#define IMGUI_BEGIN(name)
#define IMGUI_BEGIN_BOOL(name, bool)
#define IMGUI_TEXT(...)
#define IMGUI_SAMELINE(offset)
#define IMGUI_END()
#endif

#define PROFILE_LINE(line,timer) Profiler profilerVar##line(&timer)
#define PROFILE_SCOPE(timer)  PROFILE_LINE(__LINE__,timer)
#define PROFILE_FUNC(timer)   PROFILE_LINE(__LINE__,timer)

#define IMGUI_USER_CONFIG "imconfig.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"

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
		LOG_WARN("N_ButtonToString was given an unknown SDL_KeyCode, aborting");
		return "Unknown";
		break;
	};
}

#endif