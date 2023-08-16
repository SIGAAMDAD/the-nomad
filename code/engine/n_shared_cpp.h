#ifndef _N_SHARED_CPP_
#define _N_SHARED_CPP_

#pragma once

#ifdef __cplusplus

#if !defined(NDEBUG) && !defined(_NOMAD_DEBUG)
    #undef NDEBUG
#endif

namespace boost {
	template<typename mutex>
	class spinlock
	{
	private:
		eastl::atomic<bool> _lock = {false};
	public:
		inline spinlock(void) = default;
		~spinlock() = default;

		inline void lock(void)
		{
			for (;;) {
				if (!_lock.exchange(true, eastl::memory_order_acquire))
					break;
			
			#if defined(__GNUC__) || defined(_MSVC_VER)
				while (_lock.load(eastl::memory_order_relaxed))
			#ifdef __GNUC__
					__builtin_ia32_pause();
			#elif _MSVC_VER
					_mm_pause();
			#endif
			#else
				while (_lock.load(eastl::memory_order_relaxed));
			#endif
			}
		}
		inline void unlock(void)
		{ _lock.store(false, eastl::memory_order_release); }
	};
};

template<typename T, typename... Args>
inline T* construct(T *ptr, Args&&... args)
{
    ::new ((void *)ptr) T(eastl::forward<Args>(args)...);
    return ptr;
}

template<typename T>
inline T* construct(T *ptr)
{
    ::new ((void *)ptr) T();
    return ptr;
}

using json = nlohmann::json;

typedef eastl::basic_string<char> json_string;
template<typename T>
using json_vector = eastl::vector<T, GDRAllocator<T>>;

template<typename T>
struct GDRSmartPointerDeleter {
	constexpr GDRSmartPointerDeleter() = default;
	~GDRSmartPointerDeleter() = default;

	inline constexpr void operator()(T *ptr) { Z_ChangeTag((void *)ptr, TAG_PURGELEVEL); }
};

template<typename type>
using nomadunique_ptr = eastl::unique_ptr<type, GDRSmartPointerDeleter<type>>;

template<typename T, typename... Args>
inline nomadunique_ptr<T> make_nomadunique(Args&&... args)
{
	return nomadunique_ptr<T>(
		static_cast<T*>(Z_Malloc(sizeof(T), TAG_STATIC, NULL, "zalloc")(eastl::forward<Args>(args)...)));
}

// a wee little eastl thingy
namespace eastl {
	constexpr const uint32_t file_end = SEEK_END;
	constexpr const uint32_t file_cur = SEEK_CUR;
	constexpr const uint32_t file_beg = SEEK_SET;

	struct ifstream {
		inline ifstream(const ::eastl::string& filepath)
			: fp(fopen(filepath.c_str(), "rb")) { }
		inline ifstream(const char *filepath)
			: fp(fopen(filepath, "rb")) { }
		ifstream(const ifstream &) = delete;
		ifstream(ifstream &&) = delete;
		inline ~ifstream(void)
		{ ::fclose(fp); }
		inline operator bool(void) const
		{ return fp ? true : false; }
		inline size_t read(void *buffer, size_t size)
		{ return ::fread(buffer, sizeof(char), size, fp); }
		inline bool fail(void) const
		{ return fp == NULL; }
		inline bool is_open(void) const
		{ return fp != NULL; }
		inline void close(void)
		{ ::fclose(fp); }
		inline void open(const ::eastl::string& filepath)
		{ fp = ::fopen(filepath.c_str(), "wb"); }
		inline void open(const char *filepath)
		{ fp = ::fopen(filepath, "wb"); }
		inline uint64_t seekg(uint64_t offset, const uint32_t whence)
		{ return ::fseek(fp, offset, whence); }
		inline uint64_t tellg(void)
		{ return ::ftell(fp); }
		inline void rewind(void)
		{ ::rewind(fp); }
	private:
		FILE *fp = NULL;
	};

	struct ofstream {
		inline ofstream(const ::eastl::string& filepath)
			: fp(fopen(filepath.c_str(), "wb")) { }
		inline ofstream(const char *filepath)
			: fp(fopen(filepath, "wb")) { }
		ofstream(const ofstream &) = delete;
		ofstream(ofstream &&) = delete;
		inline ~ofstream(void)
		{ ::fclose(fp); }
		inline size_t write(const void *buffer, size_t size)
		{ return ::fwrite(buffer, sizeof(char), size, fp); }
		inline bool fail(void) const
		{ return fp == NULL; }
		inline bool is_open(void) const
		{ return fp != NULL; }
		inline void close(void)
		{ ::fclose(fp); }
		inline void open(const ::eastl::string& filepath)
		{ fp = ::fopen(filepath.c_str(), "wb"); }
		inline void open(const char *filepath)
		{ fp = ::fopen(filepath, "wb"); }
		inline uint64_t seekg(uint64_t offset, const uint32_t whence)
		{ return ::fseek(fp, offset, whence); }
		inline uint64_t tellg(void)
		{ return ::ftell(fp); }
		inline void rewind(void)
		{ ::rewind(fp); }
		inline void putc(int c)
		{ ::fputc(c, fp); }
	private:
		FILE *fp = NULL;
	};
	
	struct fstream {
		inline fstream(const ::eastl::string& filepath)
			: fp(fopen(filepath.c_str(), "wb+")) { }
		inline fstream(const char *filepath)
			: fp(fopen(filepath, "wb+")) { }
		fstream(const fstream &) = delete;
		fstream(fstream &&) = delete;
		inline ~fstream(void)
		{ ::fclose(fp); }
		inline size_t read(void *buffer, size_t size)
		{ return ::fread(buffer, sizeof(char), size, fp); }
		inline size_t write(const void *buffer, size_t size)
		{ return ::fwrite(buffer, sizeof(char), size, fp); }
		inline bool fail(void) const
		{ return fp == NULL; }
		inline bool is_open(void) const
		{ return fp != NULL; }
		inline void close(void)
		{ ::fclose(fp); }
		inline void open(const ::eastl::string& filepath)
		{ fp = ::fopen(filepath.c_str(), "wb+"); }
		inline void open(const char *filepath)
		{ fp = ::fopen(filepath, "wb+"); }
		inline uint64_t seekg(uint64_t offset, const uint32_t whence)
		{ return ::fseek(fp, offset, whence); }
		inline uint64_t tellg(void)
		{ return ::ftell(fp); }
		inline void rewind(void)
		{ ::rewind(fp); }
		inline void putc(int c)
		{ ::fputc(c, fp); }
		inline int getc(void)
		{ return ::fgetc(fp); }
	private:
		FILE *fp = NULL;
	};
};

template<typename type, typename alignment>
inline type* PADP(type *base, alignment align)
{
	return (type *)((void *)PAD((intptr_t)base, align));
}

#define MAX_TICRATE 333
#define MIN_TICRATE 10
#define MAX_VERT_FOV 100
#define MAX_HORZ_FOV 250
#define MSAA_OFF 0
#define MSAA_4X 1
#define MSAA_8X 2
#define MSAA_16X 3

#define N_strtobool(str) (N_stricmpn("true", (str), 4) ? (1) : (0))
template<typename type>
inline const char* N_booltostr(type b)
{
	return b ? "true" : "false";
}


// eastl overloaders
inline std::ofstream& operator<<(std::ofstream& o, const eastl::string& data)
{
	o.write(data.c_str(), data.size());
	return o;
}
inline std::ofstream& operator>>(const eastl::string& data, std::ofstream& o)
{
	return o << data;
}
inline std::ifstream& operator>>(std::ifstream& i, eastl::string& data)
{
	return eastl::getline(i, data);
}
inline std::ifstream& operator<<(eastl::string& data, std::ifstream& i)
{
	return i >> data;
}

extern int myargc;
extern char** myargv;

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
#endif

#include "../src/g_bff.h"
#include "../engine/n_map.h"

#endif
