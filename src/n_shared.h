#ifndef _N_SHARED_
#define _N_SHARED_

#pragma once

#ifndef M_PI
#define M_PI
#endif

#ifndef _NOMAD_VERSION
#   error There must be a _NOMAD_VERSION pre-defined!
#endif

#include "n_pch.h"
#include "nomaddef.h"

#ifdef NULL
#undef NULL
#endif

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

#define NULL 0

template<class T, typename... Args>
inline T* Construct(const char* name, Args&&... args)
{
	T *ptr = (T *)Z_Malloc(sizeof(T), 1, &ptr, name);
	new (ptr) T(std::forward<Args>(args)...);
	return ptr;
}
#define CONSTRUCT(class,name,...) ({class* ptr=(class*)Z_Malloc(sizeof(class),TAG_STATIC,&ptr,name);new (ptr) class(__VA_ARGS__);ptr;})

#define NOMAD_VERSION _NOMAD_VERSION
#define NOMAD_VERSION_UPDATE _NOMAD_VERSION_UPDATE
#define NOMAD_VERSION_PATCH _NOMAD_VERSION_PATCH

#if 0
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define NOMAD_LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define NOMAD_BIG_ENDIAN
#endif
#endif

typedef enum : unsigned char
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

#define ALIGN_32 __attribute__((aligned(32)))

typedef ALIGN_32 int8_t int_align8_t;
typedef ALIGN_32 int16_t int_align16_t;
typedef ALIGN_32 int32_t int_align32_t;
typedef ALIGN_32 int64_t int_align64_t;
typedef ALIGN_32 uint8_t uint_align8_t;
typedef ALIGN_32 uint16_t uint_align16_t;
typedef ALIGN_32 uint32_t uint_align32_t;
typedef ALIGN_32 uint64_t uint_align64_t;

typedef int_fast32_t point_t;

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t mat2_t[2][2];
typedef vec_t mat3_t[3][3];
typedef vec_t mat4_t[4][4];

typedef uint_fast8_t byte;
typedef byte color_t[4];
typedef float colorf_t[4];

#define BIND_EVENT_FUNC(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::foward<decltype>(args)...); }

enum : byte
{
	D_NORTH,
	D_WEST,
	D_SOUTH,
	D_EAST,

	NUMDIR,

	D_NULL
};


void N_Error(const char *err, ...) __attribute__((noreturn)) __attribute__((format(printf, 1, 2)));

class filestream
{
private:
	FILE* fp;
public:
	inline filestream(const char* filename, const char *modes)
		: fp(fopen(filename, modes))
	{
		if (!fp)
			N_Error("filestream: failed to open file %s for modes %s", filename, modes);
	}
	inline filestream(const std::string& filename, const char *modes)
		: fp(fopen(filename.c_str(), modes))
	{
		if (!fp)
			N_Error("filestream: failed to open file %s for modes %s", filename.c_str(), modes);
	}
	inline filestream(void *buffer, size_t len, const char *modes)
		: fp(fmemopen(buffer, len, modes))
	{
		if (!fp)
			N_Error("filestream: failed to create an %s buffer", modes);
	}
	inline ~filestream()
	{ fclose(fp); }
	inline filestream(const filestream &) = delete;
	inline filestream(filestream &&) = default;

	inline int getc(void) const noexcept
	{ return ::getc(fp); }
	inline size_t read(void *buffer, size_t elemsize, size_t nelem) const noexcept
	{ return fread(buffer, elemsize, nelem, fp); }
	inline void write(const void *buffer, size_t elemsize, size_t nelem) const noexcept
	{ fwrite(buffer, elemsize, nelem, fp); }
	inline FILE* get(void) noexcept
	{ return fp; }
	inline size_t size(void) noexcept {
		fseek(fp, 0L, SEEK_END);
		size_t len = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		return len;
	}
	inline void close(void) const noexcept
	{ fclose(fp); }
	inline filestream& operator=(FILE* _fp) noexcept
	{ fp = _fp; return *this; }
	inline filestream& operator=(const filestream& f) noexcept
	{ fp = f.fp; return *this; }
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
	template<typename... Args>
	void ConPrintf(fmt::format_string<Args...> fmt, Args&&... args)
	{
		fmt::print(stdout, fmt::format(fmt, std::forward<Args>(args)...)+"\n");
	}
	template<typename... Args>
	void ConError(fmt::format_string<Args...> fmt, Args&&... args)
	{
		fmt::print(stderr, fmt::format(fmt, std::forward<Args>(args)...)+"\n");
	}
	void ConFlush()
	{
		fflush(stdout);
		fflush(stderr);
	}
};

extern int myargc;
extern char** myargv;
extern Console con;

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

#define arraylen(arr) (size_t)(sizeof(arr)/sizeof(*arr))

#ifdef _MSVC_VER
#define DEBUG_BREAK() __debugbreak()
#elif defined(_GNUC_)
#define DEBUG_BREAK() __builtin_trap()
#endif

#define cvector_clib_free free
#define cvector_clib_malloc malloc
#define cvector_clib_realloc realloc
#define cvector_clib_calloc calloc
#define CVECTOR_LOGARITHMIC_GROWTH
#include "cvector.h"

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

class mutex
{
private:
	pthread_mutex_t id;
	bool is_locked;
public:
	inline mutex()
		: is_locked(false)
	{
		pthread_mutex_init(&id, NULL);
	}
	mutex(const mutex &) = delete;
	inline mutex(mutex &&) = default;
	inline ~mutex()
	{
		pthread_mutex_destroy(&id);
	}
	void lock()
	{
		if (is_locked)
			return;
		
		pthread_mutex_trylock(&id);
		is_locked = true;
	}
	void unlock()
	{
		if (!is_locked)
			return;
		
		pthread_mutex_unlock(&id);
		is_locked = false;
	}
};

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
	void create(void *_args = NULL)
	{
		if (working)
			return;
		
		pthread_create(&id, NULL, work, _args);
		LOG_INFO("launching new thread with id {}", id);
		working = true;
	}
	void join(void *_args = NULL)
	{
		if (!working)
			return;
		
		LOG_INFO("joining worker thread with id {} back to calling thread of id {}", id, pthread_self());
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

#ifdef _NOMAD_DEBUG
#define PROFILE_LINE(line,timer) Profiler profilerVar##line(&timer)
#define PROFILE_SCOPE(timer)  PROFILE_LINE(__LINE__,timer)
#define PROFILE_FUNC(timer)   PROFILE_LINE(__LINE__,timer)
#else
#define PROFILE_LINE(line,timer) 
#define PROFILE_SCOPE(timer)
#define PROFILE_FUNC(timer)
#endif

#define IMGUI_USER_CONFIG "imconfig.h"
#include <imgui/imgui.h>
//#include <imgui/backends/imgui_impl_sdl2.h>
//#include <imgui/backends/imgui_impl_sdlrenderer.h>
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
		LOG_WARN("N_ButtonToString was given an unknown SDL_KeyCode, aborting");
		return "Unknown";
		break;
	};
}

// custom algorithm faster than stdlib's sqrt, named in honor of the famous algo
float Q_root(float x);
// yep
float Q_rsqrt(float number);
float disBetweenOBJ(const glm::vec3& src, const glm::vec3& tar);
float disBetweenOBJ(const glm::vec2& src, const glm::vec2& tar);
int32_t disBetweenOBJ(const coord_t& src, const coord_t& tar);
int I_GetParm(const char* name);

template<typename T>
inline T abs(T x) { return x ? -x : x; }

#define zeroinit(dst, size) memset(dst, 0, size)

inline const char* N_booltostr(bool b)
{ return b == true ? "true" : "false"; }
inline const char* N_booltostr2(bool b)
{ return b == true ? "yes" : "no"; }
inline bool N_strtobool(const std::string& str)
{ return str == "true" ? true : false; }
inline bool N_strtobool(const char* str)
{ return strncmp(str, "true", strlen(str)) ? true : false; }
inline bool N_strtobool2(const std::string& str)
{ return str == "yes" ? true : false; }
inline bool N_strtobool2(const char* str)
{ return strncmp(str, "yes", strlen(str)) ? true : false; }

inline int Cmd_Argc(void)
{
	return myargc;
}

inline char* Cmd_Argv(int argc)
{
	if (myargc == 1)
		return myargv[0];
	if (argc > myargc)
		N_Error("Cmd_Argv: argc greater than myargc");
	
	return myargv[argc];
}

void* N_memset (void *dest, int fill, size_t count);
void* N_memcpy (void *dest, const void *src, size_t count);
bool N_memcmp (const void *ptr1, const void *ptr2, size_t count);
char* N_strcpy (char *dest, const char *src);
char* N_strncpy (char *dest, const char *src, size_t count);
size_t N_strlen (const char *str);
char *N_strrchr(char *str, char c);
void N_strcat (char *dest, char *src);
int N_strcmp (const char *str1, const char *str2);
int N_strncmp (const char *str1, const char *str2, size_t count);
int N_strncasecmp (const char *str1, const char *str2, size_t n);
int N_atoi (const char *s);
float N_atof(const char *s);
bool N_strnbcmp(const char* str1, const char* str2, size_t n);
bool N_strbcmp(const char* str1, const char* str2);

#define alloca16(x)                     alloca(((x+15)&(~15)))
#define malloc_aligned(alignment,size)  xmalloc(((size + (alignment - 1)) & ~(alignment - 1)))
#define malloc(size)            xmalloc(size)
#define free(ptr)               xfree(ptr)
#define calloc(nelem, elemsise) memset(xmalloc(nelem*elemsize),0,nelem*elemsize)
#define realloc(ptr, nsize)     xrealloc(ptr, nsize)
template<class T>
struct nomad_allocator
{
	char allocator_name[15]={0};
	nomad_allocator() noexcept { }
	nomad_allocator(const char* name = "nallocator") noexcept {
		N_strncpy(allocator_name, name, 14);
	}

	typedef T value_type;
	template<class U>
	constexpr nomad_allocator(const nomad_allocator<U>&) noexcept { }

	[[nodiscard]] inline T* allocate(std::size_t n) const {
		T* p;
		if ((p = static_cast<T*>(xmalloc(n))) != NULL)
			return p;
		
		throw std::bad_alloc();
	}
	[[nodiscard]] inline T* allocate(std::size_t& n, std::size_t& alignment, std::size_t& offset) const {
		T* p;
		if ((p = static_cast<T*>(malloc_aligned(alignment, n))) != NULL)
			return p;
		else
			throw std::bad_alloc();
		return NULL;
	}
	[[nodiscard]] inline T* allocate(std::size_t n, std::size_t alignment, std::size_t alignmentOffset, int flags) const {
		T* p;
		if ((p = static_cast<T*>(malloc_aligned(alignment, n))) != NULL)
			return p;
		else
			throw std::bad_alloc();
		return NULL;
	}
	void deallocate(void *p, std::size_t n) const noexcept {
		free(p);
	}
};

inline size_t BytesToMiB(size_t bytes)
{

}
inline size_t BytesToGiB(size_t bytes);
inline size_t BytesToKiB(size_t bytes);
inline size_t MiBToBytes(size_t mb);

template<typename T>
struct zone_deleter;

template<typename T>
using nomadvector = eastl::vector<T, nomad_allocator<T>>;
template<typename Key, typename T>
using nomad_unordered_map = eastl::unordered_map<Key, T, eastl::hash<T>, eastl::equal_to<T>, nomad_allocator<T>>;

inline void* new_realloc(void *p, size_t nsize)
{
	if (!nsize)
		N_Error("new_realloc: bad size");
	
	void *ptr = ::operator new(nsize);
	if (p) {
		memcpy(ptr, p, nsize);
		delete p;
	}
	return ptr;
}

#endif