#if 1
#include "n_shared.h"
#include "g_game.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#if 1
struct ogg_file
{
	char *curPtr;
	char *filePtr;
	size_t fileSize;
};
size_t readOGG(void *dst, size_t size1, size_t size2, void *handle)
{
	ogg_file* file = (ogg_file *)handle;
	size_t len = size1 * size2;
	if (file->curPtr + len > file->filePtr + file->fileSize)
		len = file->filePtr + file->fileSize - file->curPtr;
	
	memcpy(dst, file->curPtr, len);
	file->curPtr += len;
	return len;
}
int seekOGG(void *handle, ogg_int64_t to, int type)
{
	ogg_file *file = (ogg_file *)handle;
	switch (type) {
	case SEEK_CUR:
		file->curPtr += to;
		break;
	case SEEK_END:
		file->curPtr = file->filePtr + file->fileSize - to;
		break;
	case SEEK_SET:
		file->curPtr = file->filePtr + to;
		break;
	default:
		return -1;
		break;
	};
	if (file->curPtr < file->filePtr) {
		file->curPtr = file->filePtr;
		return -1;
	}
	if (file->curPtr > file->filePtr + file->fileSize) {
		file->curPtr = file->filePtr + file->fileSize;
		return -1;
	}
	return 0;
}
int closeOGG(void *handle) { return 0; }
long tellOGG(void *handle)
{ return (((ogg_file*)handle)->curPtr - ((ogg_file*)handle)->filePtr); }
#endif

bff_file_t* bff;

typedef enum : uint8_t
{
	LVL_START,
	LVL_END,
	SND_START,
	SND_BUFFER_START,
	SND_BUFFER_END,
	SND_END,
} opcode_t;

static const char *rdstring(void)
{
    static char buffer[BFF_STR_SIZE];
    fread(buffer, sizeof(char), BFF_STR_SIZE, bff->fp);
    return buffer;
}

void G_LoadBFF()
{
	bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff);

	bff->fp = fopen("nomadmain.bff", "rb");
	if (!bff->fp) {
		N_Error("BFF_Init: failed to open file %s", "nomadmain.bff");
	}
	
	fread(&bff->header, sizeof(bffinfo_t), 1, bff->fp);
	if (bff->header.magic != HEADER_MAGIC) {
		N_Error("BFF_Init: header wasn't the correct constant, should be %lx, got %lx",
			(uint64_t)HEADER_MAGIC, bff->header.magic);
	}

    con.ConPrintf("number of level chunks to load: {}", bff->header.numlevels);
    con.ConPrintf("number of spawn chunks to load: {}", bff->header.numspawns);
    con.ConPrintf("number of sound chunks to load: {}", bff->header.numsounds);
    con.ConPrintf("number of texture chunks to load: {}", bff->header.numtextures);
	
	bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_CACHE, &bff->levels);
	bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numsounds, TAG_CACHE, &bff->sounds);
	bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_CACHE, &bff->spawns);
    bff->textures = (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_CACHE, &bff->textures);

    Snd_Init();

    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        bff_level_t* const ptr = &bff->levels[i];
        N_memset(ptr, 0, sizeof(bff_level_t));

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading level chunk {}: {}", i, std::string(ptr->name));

        char *inbuffer;
        unsigned int insize;
        unsigned int destlen = sizeof(ptr->lvl_map);

        fread(&insize, sizeof(unsigned int), 1, bff->fp);
        inbuffer = (char *)Z_Malloc(insize, TAG_LOAD, &inbuffer);
        fread(inbuffer, sizeof(char), insize, bff->fp);
        int ret = BZ2_bzBuffToBuffDecompress((char *)ptr, &destlen, inbuffer, insize, 0, 0);
        if (ret != BZ_OK) {
            N_Error("G_LoadBFF: failed to decompress %i bytes using bzip2, error: %s", insize, bzip2_strerror(ret));
        }
        LOG_TRACE("successfully decompressed {} bytes of level data", insize);
        Z_Free(inbuffer);
    }
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
        bff_spawn_t* const ptr = &bff->spawns[i];
        N_memset(ptr, 0, sizeof(bff_spawn_t));

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        stbsp_snprintf(ptr->entityid, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading spawn chunk {}: {}", i, std::string(ptr->name));
        fread(&ptr->what, sizeof(uint8_t), 1, bff->fp);
        fread(&ptr->replacement, sizeof(sprite_t), 1, bff->fp);
        fread(&ptr->marker, sizeof(sprite_t), 1, bff->fp);
    }
    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        bff_texture_t* const ptr = &bff->textures[i];
        N_memset(ptr, 0, sizeof(bff_texture_t));
        
        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading texture chunk {}: {}", i, std::string(ptr->name));
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        ptr->buffer = (char *)Z_Malloc(ptr->fsize, TAG_LOAD, &ptr->buffer);

        char *inbuffer;
        unsigned int insize;
        unsigned int destlen = ptr->fsize;

        fread(&insize, sizeof(unsigned int), 1, bff->fp);
        inbuffer = (char *)Z_Malloc(insize, TAG_LOAD, &inbuffer);
        fread(inbuffer, sizeof(char), insize, bff->fp);
        int ret = BZ2_bzBuffToBuffDecompress(ptr->buffer, &destlen, inbuffer, insize, 0, 0);
        if (ret != BZ_OK) {
            N_Error("G_LoadBFF: failed to decompress %i bytes using bzip2, error: %s", insize, bzip2_strerror(ret));
        }
        LOG_TRACE("successfully decompressed {} bytes of texture data", insize);
    }
    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        bff_audio_t* const ptr = &bff->sounds[i];

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading audio chunk {}: {}", i, std::string(ptr->name));
        
        fread(&ptr->lvl_index, sizeof(int32_t), 1, bff->fp);
        fread(&ptr->channels, sizeof(int), 1, bff->fp);
        fread(&ptr->samplerate, sizeof(int), 1, bff->fp);
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        ptr->buffer.resize(ptr->fsize);
        fread(ptr->buffer.data(), sizeof(int16_t), ptr->fsize, bff->fp);

#if 0
        unsigned int insize;
        unsigned int destlen = ptr->fsize;
        char *inbuffer;

        fread(&insize, sizeof(unsigned int), 1, bff->fp);
        inbuffer = (char *)Z_Malloc(insize, TAG_LOAD, &inbuffer);
        fread(inbuffer, sizeof(char), insize, bff->fp);
        int ret = BZ2_bzBuffToBuffDecompress((char *)ptr->buffer.data(), &destlen, inbuffer, insize, 0, 0);
        if (ret != BZ_OK) {
            N_Error("G_LoadBFF: failed to decompress %i bytes using bzip2, error %s", insize, bzip2_strerror(ret));
        }
        LOG_TRACE("successfully decompressed {} bytes of audio data", insize);
#endif

        sfx_cache = (nomadsnd_t *)Z_Realloc(sfx_cache, sizeof(nomadsnd_t) * (i ? i : 1), &sfx_cache, TAG_CACHE);
        alGenSources(1, &sfx_cache[i].source);
        alGenBuffers(1, &sfx_cache[i].buffer);
        alBufferData(sfx_cache[i].buffer, ptr->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            &ptr->buffer.front(), ptr->buffer.size() * sizeof(int16_t), ptr->samplerate);
        alSourcei(sfx_cache[i].source, AL_BUFFER, sfx_cache[i].buffer);
        alSourcef(sfx_cache[i].source, AL_GAIN, scf::audio::sfx_vol);
    }
    alSourcePlay(sfx_cache[0].source);

    // clean up any memory left behind
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
	
	fclose(bff->fp);
}

void G_WriteBFF(const char* outfile, const char* dirname)
{
    if (!outfile)
        return;
    
    bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff);

    std::ifstream file(std::string(dirname)+"entries.json", std::ios::in);
    if (file.fail()) {
        N_Error("G_WriteBFF: failed to open entries file %s", std::string(std::string(dirname)+"entries.json").c_str());
    }
    const json data = json::parse(file);
    file.close();

    // get the header
    {
        const json header = data["header"];
        bff->header.numlevels = (uint16_t)header["numlevels"];
        bff->header.numsounds = (uint16_t)header["numsounds"];
        bff->header.numspawns = (uint16_t)header["numspawns"];
        bff->header.numtextures = (uint16_t)header["numtextures"];
        const std::string name = header["bffname"];
        bff->header.name = name.c_str();
    }

    // load the spawns
    {
        bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_STATIC, &bff->spawns);

        for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
            const std::string node_name = "spawner_"+std::to_string(i);
            bff_spawn_t* ptr = &bff->spawns[i];

            ptr->replacement = data[node_name]["replacement"];
            ptr->marker = data[node_name]["marker"];
            const std::string type = data[node_name]["entity"];
            if (type == "ET_MOB")
                ptr->what = ET_MOB;
            else if (type == "ET_PLAYR")
                ptr->what = ET_PLAYR;
            else if (type == "ET_ITEM")
                ptr->what = ET_ITEM;
            else if (type == "ET_WEAPON")
                ptr->what = ET_WEAPON;
            
            const std::string id = data[node_name]["id"];
            stbsp_snprintf(ptr->entityid, BFF_STR_SIZE, "%s", id.c_str());
            const std::string name = data[node_name]["name"];
            stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());
        }
    }
    // load the levels
    {
        bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_STATIC, &bff->levels);

        for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
            const std::string node_name = "level_"+std::to_string(i);
            const json lvl = data[node_name];
            bff_level_t* ptr = &bff->levels[i];
            const std::string name = lvl["name"];
            if (name.size() > BFF_STR_SIZE) {
                LOG_WARN("level name string size is greater than %i characters, only copying %i characters",
                    BFF_STR_SIZE, BFF_STR_SIZE);
            }
            stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());

            // load the mapfiles
            for (uint8_t m = 0; m < NUMSECTORS; ++m) {
                const std::string mapfile = lvl["mapfile_"+std::to_string(m)];
                std::ifstream file(std::string(dirname)+mapfile, std::ios::in);
                if (file.fail()) {
                    N_Error("G_WriteBFF: failed to open mapfile %s", std::string(std::string(dirname)+mapfile).c_str());
                }
                std::vector<std::string> strbuf;
                std::string line;
                while (std::getline(file, line)) { strbuf.emplace_back(line); }
                file.close();
                
                N_memset(ptr->lvl_map[m], SPR_WALL, sizeof(ptr->lvl_map[m]));
                for (uint16_t y = 0; y < strbuf.size(); ++y) {
                    for (uint16_t x = 0; x < strbuf[y].size(); ++x) {
                        sprite_t spr;
                        switch (strbuf[y][x]) {
                        case '#': spr = SPR_WALL; break;
                        case '.': spr = SPR_FLOOR_INSIDE; break;
                        case ' ': spr = SPR_FLOOR_OUTSIDE; break;
                        case '_': spr = SPR_DOOR_STATIC; break;
                        case '<': spr = SPR_DOOR_OPEN; break;
                        case '>': spr = SPR_DOOR_CLOSE; break;
                        case '&': spr = SPR_ROCK; break;
                        case ';': spr = SPR_WATER; break;
                        default: spr = SPR_CUSTOM; break;
                        };
                        ptr->lvl_map[m][y][x] = spr;
                    }
                }
                // link spawners
                for (uint16_t s = 0; s < bff->header.numspawns; ++s) {
                    bff_spawn_t* spn = &bff->spawns[s];
                    for (uint16_t y = 0; y < SECTOR_MAX_Y; ++y) {
                        for (uint16_t x = 0; x < SECTOR_MAX_X; ++x) {
                            if (ptr->lvl_map[m][y][x] == spn->marker) {
                                spn->where = {y, x};
                            }
                        }
                    }
                }
            }
        }
    }
    // load the textures
    {
        bff->textures = (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_STATIC, &bff->textures);

        for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
            const std::string node_name = "texture_"+std::to_string(i);
            const json tex = data[node_name];
            bff_texture_t* ptr = &bff->textures[i];

            const std::string name = tex["name"];
            stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());
            const std::string texfile = tex["filepath"];
            std::ifstream fp(std::string(std::string(dirname)+texfile).c_str(), std::ios::in | std::ios::binary);
            if (fp.fail()) {
                N_Error("G_WriteBFF: failed to create a readonly filestream for texture file %s", texfile.c_str());
            }
            fp.seekg(0L, std::ios_base::end);
            ptr->fsize = fp.tellg();
            fp.seekg(0L, std::ios_base::beg);
            ptr->buffer = (char *)Z_Malloc(ptr->fsize, TAG_STATIC, &ptr->buffer);
            fp.read(ptr->buffer, ptr->fsize);
        }
    }
    // load the sounds
    {
        bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_STATIC, &bff->sounds);

        for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
            const std::string node_name = "sound_"+std::to_string(i);
            const json snd = data[node_name];
            bff_audio_t* ptr = &bff->sounds[i];
            const std::string name = snd["name"];
            if (name.size() > BFF_STR_SIZE) {
                LOG_WARN("sound name size is greater than %i character, only copying up to %i characters",
                    BFF_STR_SIZE, BFF_STR_SIZE);
            }
            stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());

            const std::string sndfile = snd["filepath"];

            SF_INFO fdata;
            N_memset(&fdata, 0, sizeof(SF_INFO));
            SNDFILE* sf = sf_open(std::string(std::string(dirname)+sndfile).c_str(), SFM_READ, &fdata);
            if (!sf) {
                N_Error("G_WriteBFF: failed to open audio file %s", std::string(std::string(dirname)+sndfile).c_str());
            }
            size_t read;
            int16_t buffer[4096];
            while ((read = sf_read_short(sf, buffer, sizeof(buffer))) != 0) {
                ptr->buffer.insert(ptr->buffer.end(), buffer, buffer + read);
            }
            ptr->samplerate = fdata.samplerate;
            ptr->channels = fdata.channels;
            sf_close(sf);
            ptr->fsize = ptr->buffer.size();
            LOG_INFO("done");
        }
    }

    // write everything
    FILE* fp = fopen(outfile, "wb");
    if (!fp) {
        N_Error("G_WriteBFF: failed to open output bff file %s", outfile);
    }

    bff->header.magic = HEADER_MAGIC;
    fwrite(&bff->header, sizeof(bffinfo_t), 1, fp);
    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        fwrite(bff->levels[i].name, sizeof(char), BFF_STR_SIZE, fp);
        char outbuffer[(MAP_MAX_Y * MAP_MAX_X) / 2];
        unsigned int outsize = sizeof(outbuffer);

        int ret = BZ2_bzBuffToBuffCompress(outbuffer, &outsize, (char *)bff->levels[i].lvl_map, sizeof(bff->levels[i].lvl_map),
            9, 1, 100);
        if (ret != BZ_OK) {
            fclose(fp);
            N_Error("G_WriteBFF: bzip2 failed to compress a buffer with error %s", bzip2_strerror(ret));
        }
        
        LOG_INFO("compressed bff level data from size of {} to new size of {}, compressed {} bytes", sizeof(bff->levels[i].lvl_map), outsize,
            sizeof(bff->levels[i].lvl_map) - outsize);
        fwrite(&outsize, sizeof(unsigned int), 1, fp);
        fwrite(outbuffer, sizeof(char), outsize, fp);
    }
    Z_Free(bff->levels);
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) { // no need to compress spawners
        fwrite(bff->spawns[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(bff->spawns[i].entityid, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->spawns[i].what, sizeof(uint8_t), 1, fp);
        fwrite(&bff->spawns[i].replacement, sizeof(sprite_t), 1, fp);
        fwrite(&bff->spawns[i].marker, sizeof(sprite_t), 1, fp);
    }
    Z_Free(bff->spawns);
    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        fwrite(bff->textures[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->textures[i].fsize, sizeof(uint64_t), 1, fp);

        char *outbuffer = (char *)Z_Malloc(bff->textures[i].fsize, TAG_STATIC, &outbuffer);
        unsigned int outsize = bff->textures[i].fsize;
        
        int ret = BZ2_bzBuffToBuffCompress(outbuffer, &outsize, bff->textures[i].buffer, bff->textures[i].fsize, 9, 1, 100);
        if (ret != BZ_OK) {
            fclose(fp);
            N_Error("G_WriteBFF: bzip2 failed to compress a buffer with error %s", bzip2_strerror(ret));
        }
        LOG_INFO("compressed bff texture data from size of {} to new size of {}, compressed {} bytes", bff->textures[i].fsize, outsize,
            bff->textures[i].fsize - outsize);

        fwrite(&outsize, sizeof(unsigned int), 1, fp);
        fwrite(outbuffer, sizeof(char), outsize, fp);
        Z_Free(bff->textures[i].buffer);
        Z_Free(outbuffer);
    }
    Z_Free(bff->textures);
    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        fwrite(bff->sounds[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->sounds[i].lvl_index, sizeof(int32_t), 1, fp);
        fwrite(&bff->sounds[i].channels, sizeof(int), 1, fp);
        fwrite(&bff->sounds[i].samplerate, sizeof(int), 1, fp);
        fwrite(&bff->sounds[i].fsize, sizeof(uint64_t), 1, fp);
        fwrite(bff->sounds[i].buffer.data(), sizeof(int16_t), bff->sounds[i].buffer.size(), fp);
#if 0
        char *outbuffer = (char *)Z_Malloc(bff->sounds[i].fsize, TAG_STATIC, &outbuffer);
        unsigned int outsize = bff->sounds[i].fsize;

        int ret = BZ2_bzBuffToBuffCompress(outbuffer, &outsize, (char *)bff->sounds[i].buffer.data(), bff->sounds[i].fsize, 9, 1, 100);
        if (ret != BZ_OK) {
            fclose(fp);
            N_Error("G_LoadBFF: bzip2 failed to compress a buffer with error %s", bzip2_strerror(ret));
        }
        LOG_INFO("compressed bff audio data from size of {} to new size of {}, compressed {} bytes", bff->sounds[i].fsize, outsize,
            bff->sounds[i].fsize - outsize);
        fwrite(&outsize, sizeof(unsigned int), 1, fp);
        fwrite(outbuffer, sizeof(char), outsize, fp);
        Z_Free(outbuffer);
#endif
        bff->sounds[i].buffer.clear();
    }
    fclose(fp);

    exit(EXIT_SUCCESS);
}

#else

#include "n_shared.h"
#include "g_game.h"
#include <zlib.h>

bff_file_t* bff;

typedef enum : uint8_t
{
	LVL_START,
	LVL_END,
	SND_START,
	SND_BUFFER_START,
	SND_BUFFER_END,
	SND_END,
} opcode_t;

static void I_CacheLevels(bff_level_t* const levels);
static void I_CacheSpawns(bff_spawn_t* const spawns);
static void I_CacheTextures(bff_texture_t* const textures);
static void I_CacheAudio(bff_audio_t* const sounds);

static void I_FetchSpawns(const json& data);
static void I_FetchLevels(const json& data);
static void I_FetchTextures(const json& data);
static void I_FetchSounds(const json& data);

inline const char* zlib_strerror(int err)
{
	switch (err) {
	case Z_OK: return "Z_OK: how... something on your side is broken";
	case Z_STREAM_ERROR: return "Z_STREAM_ERROR: compression level provided was somehow invalid";
	case Z_MEM_ERROR: return "Z_MEM_ERROR: zlib couldn't allocate required memory to compress the buffer";
	case Z_BUF_ERROR: return "Z_BUF_ERROR: buffer overflow occurred";
	};
}

static void G_DecompressBuffer(char *dst, uint64_t *dstlen)
{
	char *inbuffer;
	uint64_t insize;

	fread(&insize, sizeof(uint64_t), 1, bff->fp);
	inbuffer = (char *)Z_Malloc(insize, TAG_LOAD, &inbuffer);
	fread(inbuffer, sizeof(char), insize, bff->fp);
	
	if (bff->header.compression == 0) {
		int ret = BZ2_bzBuffToBuffDecompress(dst, (unsigned int *)dstlen, inbuffer, (unsigned int)insize, 0, 0);
		if (ret != BZ_OK) {
			fclose(bff->fp);
			N_Error("G_LoadBFF: failed to decompress %ld bytes using bzip2, error: %s",
				insize, bzip2_strerror(ret));
		}
	}
	else if (bff->header.compression == 1) {
		int ret = uncompress2((Bytef *)dst, (uLongf *)dstlen, (Bytef *)inbuffer, (uLong *)&insize);
		if (ret != Z_OK) {
			fclose(bff->fp);
			N_Error("G_LoadBFF: failed to decompress %ld bytes using zlib, error: %s",
				insize, zlib_strerror(ret));
		}
	}
	LOG_TRACE("successfully decompressed {} bytes from buffer", insize);
}


static void G_CompressBuffer(char *src, uint64_t srclen, FILE* fp)
{
	char *outbuffer = (char *)Z_Malloc(srclen, TAG_LOAD, &outbuffer);
    uint64_t outlen;
	
	if (bff->header.compression == 0) {
		int ret = BZ2_bzBuffToBuffCompress(outbuffer, (unsigned int *)&outlen, src, (unsigned int)srclen, 9, 1, 100);
		if (ret != BZ_OK) {
			fclose(fp);
			N_Error("G_WriteBFF: failed to compress buffer of size %ld, bzip2 error: %s",
				srclen, bzip2_strerror(ret));
        }
	}
	else if (bff->header.compression == 1) {
		int ret = compress2((Bytef *)outbuffer, (uLongf *)&outlen, (Bytef *)src, (uLong)srclen, Z_BEST_COMPRESSION);
		if (ret != Z_OK) {
			fclose(fp);
			N_Error("G_WriteBFF: failed to compress buffer of size %ld, zlib error: %s",
				srclen, zlib_strerror(ret));
		}
	}
	LOG_INFO("compressed buffer from size of {} to new size of {}, compressed {} bytes", srclen, outlen,
        srclen - outlen);
	fwrite(&outlen, sizeof(uint64_t), 1, fp);
	fwrite(outbuffer, sizeof(char), outlen, fp);
}

static const char *rdstring(void)
{
    static char buffer[BFF_STR_SIZE];
    fread(buffer, sizeof(char), BFF_STR_SIZE, bff->fp);
    return buffer;
}

void G_LoadBFF()
{
	bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff);

	bff->fp = fopen("nomadmain.bff", "rb");
	if (!bff->fp) {
		N_Error("BFF_Init: failed to open file %s", "nomadmain.bff");
	}
	
	fread(&bff->header, sizeof(bffinfo_t), 1, bff->fp);
	if (bff->header.magic != HEADER_MAGIC) {
		N_Error("BFF_Init: header wasn't the correct constant, should be %lx, got %lx",
			(uint64_t)HEADER_MAGIC, bff->header.magic);
	}

    con.ConPrintf("number of level chunks to load: {}", bff->header.numlevels);
    con.ConPrintf("number of spawn chunks to load: {}", bff->header.numspawns);
    con.ConPrintf("number of sound chunks to load: {}", bff->header.numsounds);
    con.ConPrintf("number of texture chunks to load: {}", bff->header.numtextures);
	
	bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_CACHE, &bff->levels);
	bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numsounds, TAG_CACHE, &bff->sounds);
	bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_CACHE, &bff->spawns);
    bff->textures = (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_CACHE, &bff->textures);

    Snd_Init();
    I_CacheLevels(bff->levels);
    I_CacheSpawns(bff->spawns);
    I_CacheTextures(bff->textures);
    I_CacheAudio(bff->sounds);
    
    // clean up any memory left behind
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
	
	fclose(bff->fp);
}

static const char *dirname;

void G_WriteBFF(const char* outfile, const char* dir, int compression)
{
    if (!outfile || !dirname)
        exit(EXIT_FAILURE);
    
    dirname = dir;
    bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff);

    std::ifstream file(std::string(dirname)+"entries.json", std::ios::in);
    if (file.fail()) {
        N_Error("G_WriteBFF: failed to open entries file %s", std::string(std::string(dirname)+"entries.json").c_str());
    }
    const json data = json::parse(file);
    file.close();

    // get the header
    {
        const json header = data["header"];
        bff->header.numlevels = (uint16_t)header["numlevels"];
        bff->header.numsounds = (uint16_t)header["numsounds"];
        bff->header.numspawns = (uint16_t)header["numspawns"];
        bff->header.numtextures = (uint16_t)header["numtextures"];
        const std::string name = header["bffname"];
        N_strncpy(bff->header.name, name.c_str(), 256);
        bff->header.compression = compression;
    }
    
    I_FetchSpawns(data);
    I_FetchLevels(data);
    I_FetchTextures(data);
    I_FetchSounds(data);

    // write everything
    FILE* fp = fopen(outfile, "wb");
    if (!fp) {
        N_Error("G_WriteBFF: failed to open output bff file %s", outfile);
    }

    bff->header.magic = HEADER_MAGIC;
    fwrite(&bff->header, sizeof(bffinfo_t), 1, fp);
    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        fwrite(bff->levels[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->levels[i].numspawns, sizeof(uint16_t), 1, fp);
        fwrite(bff->levels[i].rewards, sizeof(uint32_t), NUMREWARDS, fp);
        fwrite(bff->levels[i].spawnlist, sizeof(uint16_t), bff->levels[i].numspawns, fp);
        G_CompressBuffer((char *)bff->levels[i].lvl_map, MAP_MAX_Y * MAP_MAX_X, fp);
    }
    Z_ChangeTag(bff->levels, TAG_LOAD);
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) { // no need to compress spawners
        fwrite(bff->spawns[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(bff->spawns[i].entityid, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->spawns[i].what, sizeof(uint8_t), 1, fp);
        fwrite(&bff->spawns[i].replacement, sizeof(sprite_t), 1, fp);
        fwrite(&bff->spawns[i].marker, sizeof(sprite_t), 1, fp);
    }
    Z_Free(bff->spawns);
    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        fwrite(bff->textures[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->textures[i].fsize, sizeof(uint64_t), 1, fp);
        
        G_CompressBuffer(bff->textures[i].buffer, bff->textures[i].fsize, fp);
    }
    Z_ChangeTag(bff->textures, TAG_LOAD);
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        fwrite(bff->sounds[i].name, sizeof(char), BFF_STR_SIZE, fp);
        fwrite(&bff->sounds[i].lvl_index, sizeof(int32_t), 1, fp);
//        fwrite(&bff->sounds[i].channels, sizeof(int), 1, fp);
//        fwrite(&bff->sounds[i].samplerate, sizeof(int), 1, fp);
		fwrite(&bff->sounds[i].fsize, sizeof(uint64_t), 1, fp);	
		G_CompressBuffer((char *)bff->sounds[i].buffer.data(), bff->sounds[i].fsize, fp);
		Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
    }
    fclose(fp);

    exit(EXIT_SUCCESS);
}

static void I_FetchTextures(const json& data)
{
	bff->textures = (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_STATIC, &bff->textures);

    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        const std::string node_name = "texture_"+std::to_string(i);
        const json tex = data[node_name];
        bff_texture_t* ptr = &bff->textures[i];

	    const std::string name = tex["name"];
	    stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());
	    const std::string texfile = tex["filepath"];
	    std::ifstream fp(std::string(std::string(dirname)+texfile).c_str(), std::ios::in | std::ios::binary);
	    if (fp.fail()) {
	        N_Error("G_WriteBFF: failed to create a readonly filestream for texture file %s", texfile.c_str());
	    }
	    fp.seekg(0L, std::ios_base::end);
	    ptr->fsize = fp.tellg();
	    fp.seekg(0L, std::ios_base::beg);
	    ptr->buffer = (char *)Z_Malloc(ptr->fsize, TAG_STATIC, &ptr->buffer);
	    fp.read(ptr->buffer, ptr->fsize);
	}
}

static void I_FetchSpawns(const json& data)
{
	bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_STATIC, &bff->spawns);
	
	for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
		const std::string node_name = "spawner_"+std::to_string(i);
		bff_spawn_t* ptr = &bff->spawns[i];
		
		ptr->replacement = data[node_name]["replacement"];
		ptr->marker = data[node_name]["marker"];
		const std::string type = data[node_name]["entity"];
		if (type == "ET_MOB")
			ptr->what = ET_MOB;
		else if (type == "ET_PLAYR")
			ptr->what = ET_PLAYR;
		else if (type == "ET_ITEM")
		    ptr->what = ET_ITEM;
		else if (type == "ET_WEAPON")
		    ptr->what = ET_WEAPON;
		
		const std::string id = data[node_name]["id"];
		stbsp_snprintf(ptr->entityid, BFF_STR_SIZE, "%s", id.c_str());
		const std::string name = data[node_name]["name"];
		stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());
	}
}

static void I_FetchLevels(const json& data)
{
	bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_STATIC, &bff->levels);
	
	for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
		const std::string node_name = "level_"+std::to_string(i);
		const json lvl = data[node_name];
		bff_level_t* ptr = &bff->levels[i];
		const std::string name = lvl["name"];
		if (name.size() > BFF_STR_SIZE) {
		    LOG_WARN("level name string size is greater than %i characters, only copying %i characters",
		        BFF_STR_SIZE, BFF_STR_SIZE);
		}
		stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());
		
		ptr->rewards[REWARD_GOLD] = lvl.contains("rewards_gold") ? (uint32_t)lvl["rewards_gold"] : 0;
		ptr->rewards[REWARD_XP] = lvl.contains("rewards_xp") ? (uint32_t)lvl["rewards_xp"] : 0;
		ptr->numspawns = lvl["spawns"].size();
		std::vector<std::string> spawnlist = lvl["spawns"];
		ptr->spawnlist = (uint16_t *)Z_Malloc(sizeof(uint16_t) * ptr->numspawns, TAG_STATIC, &ptr->spawnlist);
		
		for (uint16_t s = 0; s < ptr->numspawns; ++s) {
			bff_spawn_t* spawner = NULL;
			uint16_t b;
			for (b = 0; b < bff->header.numspawns; ++b) {
				if (spawnlist[s] == bff->spawns[b].entityid) {
					spawner = &bff->spawns[b];
				}
			}
			if (!spawner) {
				N_Error("G_WriteBFF: failed to find spawner to match at index %i, name: %s",
					s, spawnlist[s].c_str());
			}
			ptr->spawnlist[s] = b;
		}

		// load the mapfiles
		for (uint8_t m = 0; m < NUMSECTORS; ++m) {
			const std::string mapfile = lvl["mapfile_"+std::to_string(m)];
			std::ifstream file(std::string(dirname)+mapfile, std::ios::in);
			if (file.fail()) {
			    N_Error("G_WriteBFF: failed to open mapfile %s", std::string(std::string(dirname)+mapfile).c_str());
			}
			std::vector<std::string> strbuf;
			std::string line;
			while (std::getline(file, line)) { strbuf.emplace_back(line); }
			file.close();
			
			N_memset(ptr->lvl_map[m], SPR_WALL, sizeof(ptr->lvl_map[m]));
			for (uint16_t y = 0; y < strbuf.size(); ++y) {
			    for (uint16_t x = 0; x < strbuf[y].size(); ++x) {
			        sprite_t spr;
			        switch (strbuf[y][x]) {
			        case '#': spr = SPR_WALL; break;
			        case '.': spr = SPR_FLOOR_INSIDE; break;
			        case ' ': spr = SPR_FLOOR_OUTSIDE; break;
			        case '_': spr = SPR_DOOR_STATIC; break;
			        case '<': spr = SPR_DOOR_OPEN; break;
			        case '>': spr = SPR_DOOR_CLOSE; break;
			        case '&': spr = SPR_ROCK; break;
			        case ';': spr = SPR_WATER; break;
			        default: spr = SPR_CUSTOM; break;
			        };
			        ptr->lvl_map[m][y][x] = spr;
			    }
			}
			// link spawners
			for (uint16_t s = 0; s < bff->header.numspawns; ++s) {
			    bff_spawn_t* spn = &bff->spawns[s];
			    for (uint16_t y = 0; y < SECTOR_MAX_Y; ++y) {
			        for (uint16_t x = 0; x < SECTOR_MAX_X; ++x) {
			            if (ptr->lvl_map[m][y][x] == spn->marker) {
			                spn->where = {y, x};
			            }
			        }
			    }
			}
		}
	}
}

static void I_FetchSounds(const json& data)
{
	bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_STATIC, &bff->sounds);
	for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
		const std::string node_name = "sound_"+std::to_string(i);
		const json snd = data[node_name];
		bff_audio_t* ptr = &bff->sounds[i];
		const std::string name = snd["name"];
		if (name.size() > BFF_STR_SIZE) {
		    LOG_WARN("sound name size is greater than {} character, only copying up to {} characters",
		        BFF_STR_SIZE, BFF_STR_SIZE);
		}
		stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", name.c_str());

        const std::string sndfile = snd["filepath"];
		
		LOG_INFO("loading audio file {}", std::string(dirname)+sndfile);
#if 1
        SF_INFO fdata;
        N_memset(&fdata, 0, sizeof(SF_INFO));
        SNDFILE* sf = sf_open(std::string(std::string(dirname)+sndfile).c_str(), SFM_READ, &fdata);
        if (!sf) {
            N_Error("G_WriteBFF: failed to open audio file %s", std::string(std::string(dirname)+sndfile).c_str());
        }
        size_t read;
        int16_t buffer[4096];
        while ((read = sf_read_short(sf, buffer, sizeof(buffer))) != 0) {
            ptr->buffer.insert(ptr->buffer.end(), buffer, buffer + read);
        }
        ptr->samplerate = fdata.samplerate;
        ptr->channels = fdata.channels;
        sf_close(sf);
        ptr->fsize = ptr->buffer.size();
#else
		FILE* fp = fopen(std::string(std::string(dirname)+sndfile).c_str(), "rb");
		if (!fp) {
			N_Error("G_WriteBFF: failed to open audio file %s", std::string(std::string(dirname)+sndfile).c_str());
		}
		fseek(fp, 0L, SEEK_END);
		ptr->fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		ptr->buffer.resize(ptr->fsize / sizeof(int16_t));
		fread(ptr->buffer.data(), sizeof(char), ptr->fsize, fp);
		fclose(fp);
#endif
        LOG_INFO("done");
	}
}

static void I_LinkSpawners(bff_level_t* const ptr)
{
	for (uint16_t y = 0; y < MAP_MAX_Y; ++y) {
		for (uint16_t x = 0; x < MAP_MAX_X; ++x) {
			
		}
	}
}

static void I_CacheLevels(bff_level_t* const levels)
{
	for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        bff_level_t* const ptr = &levels[i];
        N_memset(ptr, 0, sizeof(bff_level_t));

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading level chunk {}: {}", i, std::string(ptr->name));
        
        fread(ptr->rewards, sizeof(uint32_t), NUMREWARDS, bff->fp);
        fread(&ptr->numspawns, sizeof(uint16_t), 1, bff->fp);
        ptr->spawnlist = (uint16_t *)Z_Malloc(sizeof(uint16_t) * ptr->numspawns, TAG_CACHE, &ptr->spawnlist);
        fread(ptr->spawnlist, sizeof(uint16_t), ptr->numspawns, bff->fp);

        uint64_t dstlen = sizeof(ptr->lvl_map);
        G_DecompressBuffer((char *)ptr->lvl_map, &dstlen);
    }
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
}

static void I_CacheSpawns(bff_spawn_t* const spawns)
{
	for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
        bff_spawn_t* const ptr = &spawns[i];
        N_memset(ptr, 0, sizeof(bff_spawn_t));

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        stbsp_snprintf(ptr->entityid, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading spawn chunk {}: {}", i, std::string(ptr->name));
        fread(&ptr->what, sizeof(uint8_t), 1, bff->fp);
        fread(&ptr->replacement, sizeof(sprite_t), 1, bff->fp);
        fread(&ptr->marker, sizeof(sprite_t), 1, bff->fp);
    }
}

static void I_CacheTextures(bff_texture_t* const textures)
{
	for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        bff_texture_t* const ptr = &bff->textures[i];
        N_memset(ptr, 0, sizeof(bff_texture_t));
        
        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading texture chunk {}: {}", i, std::string(ptr->name));
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        ptr->buffer = (char *)Z_Malloc(ptr->fsize, TAG_LOAD, &ptr->buffer);

        uint64_t destlen = ptr->fsize;
        G_DecompressBuffer(ptr->buffer, &destlen);
    }
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
}

static void I_CacheAudio(bff_audio_t* const sounds)
{
	for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        bff_audio_t* const ptr = &sounds[i];

        stbsp_snprintf(ptr->name, BFF_STR_SIZE, "%s", rdstring());
        con.ConPrintf("loading audio chunk {}: {}", i, std::string(ptr->name));
        
        fread(&ptr->lvl_index, sizeof(int32_t), 1, bff->fp);
        fread(&ptr->channels, sizeof(int), 1, bff->fp);
        fread(&ptr->samplerate, sizeof(int), 1, bff->fp);
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        ptr->buffer.resize(ptr->fsize);

#if 1
		uint64_t dstlen = ptr->fsize;
        G_DecompressBuffer((char *)ptr->buffer.data(), &dstlen);
#else
#endif
        sfx_cache = (nomadsnd_t *)Z_Realloc(sfx_cache, sizeof(nomadsnd_t) * (i ? i : 1), &sfx_cache, TAG_CACHE);
        alGenSources(1, &sfx_cache[i].source);
        alGenBuffers(1, &sfx_cache[i].buffer);
        alBufferData(sfx_cache[i].buffer, ptr->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            &ptr->buffer.front(), ptr->buffer.size() * sizeof(int16_t), ptr->samplerate);
        alSourcei(sfx_cache[i].source, AL_BUFFER, sfx_cache[i].buffer);
        alSourcef(sfx_cache[i].source, AL_GAIN, scf::audio::sfx_vol);
       	ptr->buffer.clear(); // its uncompressed wav, so...
    }
    Z_FreeTags(TAG_PURGELEVEL, TAG_LOAD);
}
#endif