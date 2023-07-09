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

#define MAP_MAX_Y 240
#define MAP_MAX_X 240
#endif

#define BFF_VERSION_MAJOR 0
#define BFF_VERSION_MINOR 1
#define BFF_VERSION ((BFF_VERSION_MAJOR<<8)+BFF_VERSION_MINOR)

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

#ifndef _NOMAD_VERSION
void __attribute__((noreturn)) BFF_Error(const char* fmt, ...);
#else
#define BFF_Error N_Error
#endif

const __inline int32_t bffPaidID[70] = {
	0x0000, 0x0000, 0x0412, 0x0000, 0x0000, 0x0000, 0x0527,
	0x421f, 0x0400, 0x0000, 0x0383, 0x0000, 0x0000, 0x0000,
	0x0000, 0x5666, 0x6553, 0x0000, 0x0000, 0xad82, 0x8270,
	0x2480, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x053d, 0x0000, 0x0000, 0x7813, 0x0000,
	0x02aa, 0x0100, 0x0000, 0x4941, 0x0000, 0x3334, 0xff42,
	0x0000, 0x0546, 0xa547, 0x1321, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0xa235, 0xfa31, 0x215a, 0x0000
};

constexpr uint32_t MAGIC_XOR = 0x4ff3ade3;

constexpr uint32_t MAX_BFF_PATH = 256;
constexpr uint32_t MAX_BFF_CHUNKNAME = 11;
constexpr uint32_t MAX_TEXTURE_CHUNKS = 128;
constexpr uint32_t MAX_LEVEL_CHUNKS = 128;
constexpr uint32_t MAX_SOUND_CHUNKS = 128;
constexpr uint32_t MAX_SCRIPT_CHUNKS = 64;
constexpr uint32_t MAX_MAP_SPAWNS = 1024;
constexpr uint32_t MAX_MAP_LIGHTS = 1024;

enum : int32_t {
	COMPRESSION_NONE,
	COMPRESSION_BZIP2,
	COMPRESSION_ZLIB
};

enum : int32_t
{
	SFT_OGG,
	SFT_WAV,
	SFT_OPUS
};

enum : int32_t
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
	
	int32_t fileSize;
	int32_t fileType;
} bffsound_t;

typedef struct
{
	// bff stuff
	char name[MAX_BFF_CHUNKNAME];
	int32_t levelNumber;

	char *tmjBuffer;
	char **tsjBuffers;

	int32_t mapBufferLen;
	int32_t numTilesets;
} bfflevel_t;

// scripted encounters (boss fights, story mode, etc.)
typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	
	int32_t codelen;
	uint8_t* bytecode; // q3vm raw bytecode
} bffscript_t;

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];

	int32_t fileSize;
	int32_t fileType;
	unsigned char *fileBuffer;
} bfftexture_t;


typedef struct bffinfo_s
{
	bfftexture_t textures[MAX_TEXTURE_CHUNKS];
	bfflevel_t levels[MAX_LEVEL_CHUNKS];
	bffscript_t scripts[MAX_SCRIPT_CHUNKS];
	bffsound_t sounds[MAX_SOUND_CHUNKS];

	char bffPathname[MAX_BFF_PATH];
	char bffGamename[256];
	
	int32_t compression;
	int32_t numTextures;
	int32_t numLevels;
	int32_t numSounds;
	int32_t numScripts;
} bffinfo_t;

typedef struct
{
	char chunkName[MAX_BFF_CHUNKNAME];
	int32_t chunkSize;
	char *chunkBuffer;
	int32_t chunkType;
} bff_chunk_t;

typedef struct
{
	int32_t ident = BFF_IDENT;
	int32_t magic = HEADER_MAGIC;
	int32_t numChunks;
	int32_t compression;
	int16_t version;
} bffheader_t;

typedef struct
{
	char bffPathname[MAX_BFF_PATH];
	char bffGamename[256];
	bffheader_t header;
	
	int32_t numChunks;
	bff_chunk_t* chunkList;
} bff_t;

void B_GetChunk(const char *chunkname);
void BFF_CloseArchive(bff_t* archive);
bff_t* BFF_OpenArchive(const GDRStr& filepath);
bffinfo_t* BFF_FetchInfo(void);
void BFF_FreeInfo(bffinfo_t* info);

bffscript_t* BFF_FetchScript(const char *name);
bfflevel_t* BFF_FetchLevel(const char *name);

void G_LoadBFF(const GDRStr& bffname);

#endif