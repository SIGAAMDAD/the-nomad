#ifndef _G_BFF_
#define _G_BFF_

#pragma once

constexpr uint16_t MAP_MAX_Y = 480;
constexpr uint16_t MAP_MAX_X = 480;
constexpr uint8_t NUMSECTORS = 4;
constexpr uint8_t SECTOR_MAX_Y = 120;
constexpr uint8_t SECTOR_MAX_X = 120;

#ifdef _WIN32
#define open(name, flags, mode) _open(name, flags, mode)
#define write(handle, buffer, size) _write(handle, buffer, size)
#define read(handle, buffer, size) _read(handle, buffer, size)
#define close(handle) _close(handle)
#endif

inline size_t filesize(const char* filepath)
{
#ifdef __unix__
	struct stat fdata;
	if (stat(filepath, &fdata) == -1)
#elif defined(_WIN32)
	struct _stati64 fdata;
	if (_stati64(filepath, &fdata) == -1)
#endif
    {
		N_Error("filesize: failed to stat() file %s", filepath);
	}
	return fdata.st_size;
}

typedef struct bff_level_s bff_level_t;

#define HEADER_MAGIC 0x5f3759df

typedef struct bff_spawn_s
{
	char name[80];
	uint16_t replacement;
	uint16_t marker;
    coord_t where;
	uint8_t what;
	
	void *heap;
} bff_spawn_t;

typedef struct bff_audio_s
{
	char name[80];
	int32_t lvl_index; // equal to -1 if not a level-specific track
	uint64_t fsize;
    int32_t channels;
    int32_t samplerate;
    bool is_sfx;

    uint64_t numbuffers;
	int16_t *buffer;
	bff_level_t* level;
} bff_audio_t;

typedef struct bff_level_s
{
	char name[80];
	uint16_t lvl_map[NUMSECTORS][SECTOR_MAX_Y][SECTOR_MAX_X];
	
	uint16_t numspawns;
	uint16_t numtracks;
	bff_audio_t *tracks;
	bff_spawn_t *spawns;
} bff_level_t;

typedef struct bffinfo_s
{
	uint64_t magic = HEADER_MAGIC;
	char name[80];
	uint16_t numlevels;
	uint16_t numspawns;
	uint16_t numsounds;
} bffinfo_t;

typedef struct bff_file_s
{
	FILE* fp;
	bffinfo_t header;
	bff_spawn_t* spawns;
	bff_audio_t* sounds;
	bff_level_t* levels;
} bff_file_t;

extern bff_file_t* bff;

void G_LoadBFF(const char* filename);
void G_WriteBFF(const char* outfile, const char* dirname);

#endif