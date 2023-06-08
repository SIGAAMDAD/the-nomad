#ifndef _G_BFF_
#define _G_BFF_

#pragma once
#if 0
#ifdef __BIG_ENDIAN__
inline void SwapBytes(void *pv, size_t n)
{
	assert(n > 0);
	char *p = (char *)pv;
	size_t lo, hi;
	
	for (lo = 0, hi = n - 1; hi > lo; lo++, hi--) {
		char tmp = p[lo];
		p[lo] = p[hi];
		p[hi] = tmp;
	}
}
#define SWAP(x) SwapBytes(&x, sizeof(x))
#else
#define SWAP(x)
#endif

#define NUMSECTORS 4
#define SECTOR_MAX_Y 120
#define SECTOR_MAX_X 120
#define MAP_MAX_Y 240
#define MAP_MAX_X 240
#endif

#define HEADER_MAGIC 0x5f3759df
#define BFF_IDENT (('B'<<24)+('F'<<16)+('F'<<8)+'I')
#define BFF_STR_SIZE (int)80
#define DEFAULT_BFF_GAMENAME "bffNoName"

enum : uint64_t
{
	LEVEL_CHUNK,
	SOUND_CHUNK,
	SCRIPT_CHUNK,
	TEXTURE_CHUNK
};

enum : char
{
	SPR_SPAWNER = '$',
	SPR_LAMP = '|',
};

typedef uint32_t bff_int_t;
typedef uint16_t bff_short_t;
typedef float bff_float_t;

#ifndef _NOMAD_VERSION
void __attribute__((noreturn)) BFF_Error(const char* fmt, ...);
#else
#define BFF_Error N_Error
#endif

const __inline uint16_t bffPaidID[70] = {
	0x0000, 0x0000, 0x0412, 0x0000, 0x0000, 0x0000, 0x0527,
	0x421f, 0x0400, 0x0000, 0x0383, 0x0000, 0x0000, 0x0000,
	0x0000, 0x5666, 0x6553, 0x0000, 0x0000, 0xad82, 0x8270,
	0x2480, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x053d, 0x0000, 0x0000, 0x7813, 0x0000,
	0x02aa, 0x0100, 0x0000, 0x4941, 0x0000, 0x3334, 0xff42,
	0x0000, 0x0546, 0xa547, 0x1321, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xa235, 0xfa31, 0x215a, 0x0000
};

#define MAGIC_XOR 0x4ff3ade3

#define MAX_BFF_PATH 256
#define MAX_BFF_CHUNKNAME 11
#define MAX_TEXTURE_CHUNKS 128
#define MAX_LEVEL_CHUNKS 128
#define MAX_SOUND_CHUNKS 128
#define MAX_SCRIPT_CHUNKS 64
#define MAX_MAP_SPAWNS 1024
#define MAX_MAP_LIGHTS 1024

enum : uint16_t
{
	SFT_OGG,
	SFT_WAV,
	SFT_OPUS
};

enum : bff_short_t
{
	TEX_JPG,
	TEX_BMP,
	TEX_TGA,
	TEX_PNG,
};

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	char *fileBuffer;
	bff_int_t fileSize;
	bff_short_t fileType;
} bffsound_t;

typedef struct
{
	bff_int_t y;
	bff_int_t x;
	bff_short_t sector;
} mapspawn_t;

typedef struct
{
	bff_int_t y;
	bff_int_t x;
	bff_short_t sector;
	bff_float_t intensity;
	bff_float_t aoe;
} maplight_t;

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	char tilemap[NUMSECTORS][SECTOR_MAX_Y][SECTOR_MAX_X];
	mapspawn_t spawns[MAX_MAP_SPAWNS];
	maplight_t lights[MAX_MAP_LIGHTS];
	bff_int_t levelNumber;
	bff_int_t numLights;
	bff_int_t numSpawns;
} bfflevel_t;

// scripted encounters (boss fights, story mode, etc.)
typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	
	bff_int_t codelen;
	uint8_t* bytecode; // q3vm raw bytecode
} bffscript_t;

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];

	bff_int_t fileSize;
	bff_short_t fileType;
	unsigned char *fileBuffer;
} bfftexture_t;

typedef struct
{
	bff_float_t lightIntensity;
	bff_float_t lightAOE;
} bffconfig_t;

typedef struct bffinfo_s
{
	// compile-time variables, not written to the bff
	bffconfig_t config;

	bfftexture_t textures[MAX_TEXTURE_CHUNKS];
	bfflevel_t levels[MAX_LEVEL_CHUNKS];
	bffscript_t scripts[MAX_SCRIPT_CHUNKS];
	bffsound_t sounds[MAX_SOUND_CHUNKS];

	char bffPathname[MAX_BFF_PATH];
	char bffGamename[256];
	
	bff_int_t numTextures;
	bff_int_t numLevels;
	bff_int_t numSounds;
	bff_int_t numScripts;
} bffinfo_t;

typedef struct
{
	char chunkName[MAX_BFF_CHUNKNAME];
	bff_int_t chunkSize;
	char *chunkBuffer;
	bff_int_t chunkType;
} bff_chunk_t;

typedef struct
{
	bff_int_t ident = BFF_IDENT;
	bff_int_t magic = HEADER_MAGIC;
	bff_int_t numChunks;
} bffheader_t;

typedef struct
{
	char bffPathname[MAX_BFF_PATH];
	char bffGamename[256];
	
	bff_int_t numChunks;
	bff_chunk_t* chunkList;
} bff_t;

void B_GetChunk(const char *chunkname);
void BFF_CloseArchive(bff_t* archive);
bff_t* BFF_OpenArchive(const GDRStr& filepath);

bffscript_t* BFF_FetchScript(const char *name);

void G_LoadBFF(const GDRStr& bffname);

#endif