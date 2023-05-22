#ifndef _G_BFF_
#define _G_BFF_

#pragma once

#ifdef QVM
#error only compile bff_file on native builds!
#endif

#ifndef _NOMAD_VERSION // if this isn't defined, then we're compiling the bff-tool program
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <string.h>
#include <strings.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

#define HEADER_MAGIC 0x5f3759df
#define BFF_IDENT (('B'<<24)+('F'<<16)+('F'<<8)+'I')
#define BFF_STR_SIZE (int)80
#define DEFAULT_BFF_GAMENAME "bffNoName"

enum : uint64_t
{
	LEVEL_CHUNK,
	SOUND_CHUNK,
	SCRIPT_CHUNK
};


#ifndef _NOMAD_VERSION
void __attribute__((noreturn)) BFF_Error(const char* fmt, ...);
#else
#define BFF_Error N_Error
#endif

#define MAX_BFF_PATH 256
#define MAX_BFF_CHUNKNAME 11
#define MAX_TEXTURE_CHUNKS 128
#define MAX_LEVEL_CHUNKS 128
#define MAX_SOUND_CHUNKS 128
#define MAX_SCRIPT_CHUNKS 64
#define MAX_MAP_SPAWNS 1024
#define MAX_MAP_LIGHTS 1024

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	char *filebuffer;
	uint64_t filesize;
	uint16_t filetype;
} bffsound_t;

typedef struct
{
	uint32_t y;
	uint32_t x;
	uint16_t sector;
} mapspawn_t;

typedef struct
{
	uint32_t y;
	uint32_t x;
	uint16_t sector;
	float intensity;
	float aoe;
} maplight_t;

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	char tilemap[NUMSECTORS][SECTOR_MAX_Y][SECTOR_MAX_X];
	mapspawn_t spawns[MAX_MAP_SPAWNS];
	maplight_t lights[MAX_MAP_LIGHTS];
	uint64_t levelNumber;
	uint64_t numLights;
	uint64_t numSpawns;
} bfflevel_t;

// scripted encounters (boss fights, story mode, etc.)
typedef struct
{
	char name[MAX_BFF_CHUNKNAME];
	bfflevel_t* level; // level data
	bffsound_t* music; // soundtrack data
	
	uint64_t codelen;
	uint8_t* bytecode; // q3vm raw bytecode
} bffscript_t;

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];

	uint16_t fileType;
	uint64_t fileSize;
	unsigned char *fileBuffer;
} bfftexture_t;

typedef struct
{
	float lightIntensity;
	float lightAOE;
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
	
	uint64_t numTextures;
	uint64_t numLevels;
	uint64_t numSounds;
	uint64_t numScripts;
} bffinfo_t;

typedef struct
{
	char chunkName[MAX_BFF_CHUNKNAME];
	uint64_t chunkSize;
	char *chunkBuffer;
	uint32_t chunkType;
} bff_chunk_t;

typedef struct
{
	uint32_t ident = BFF_IDENT;
	uint64_t magic = HEADER_MAGIC;
	uint64_t numChunks;
	uint64_t fileDecompressedSize;
	uint64_t fileCompressedSize;
} bffheader_t;

typedef struct
{
	char bffPathname[MAX_BFF_PATH];
	char bffGamename[256];
	
	uint64_t numChunks;
	bff_chunk_t* chunkList;
} bff_t;

typedef unsigned char byte;

bff_t* BFF_OpenArchive(const eastl::string& filepath);
bffinfo_t* BFF_GetInfo(bff_t* archive);
void BFF_FreeInfo(bffinfo_t* info);
void BFF_CloseArchive(bff_t* archive);

#ifndef _NOMAD_VERSION
void DecompileBFF(const char *filepath);
void WriteBFF(const char *outfile, const char *jsonfile);
void GetLevels(const json& data, bffinfo_t* info);
void GetScripts(const json& data, bffinfo_t* info);

FILE* SafeOpen(const char* filepath, const char* mode);
void* SafeMalloc(size_t size, const char* name = "unknown");
void LoadFile(FILE* fp, void** buffer, size_t* length);

void Con_Printf(const char *fmt, ...);
#else
#define SafeOpen(filepath, mode) N_OpenFile(filepath, mode)
#define SafeMalloc(size, name) xmalloc(size)
#define LoadFile(fp, buffer, length) N_ReadFile(fp, buffer, length)
#endif

#endif