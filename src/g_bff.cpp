#include "n_shared.h"
#include "g_game.h"
#include <ogg/ogg.h>
#include <sndfile.h>
#include <vorbis/vorbisfile.h>
#include "stb_vorbis.c"

#if 0
static sf_count_t filesize_vio(void *handle)
{
    return static_cast<bff_audio_t*>(handle)->fsize;
}
static sf_count_t seek_vio(sf_count_t offset, int whence, void *handle)
{
    bff_audio_t* ptr = (bff_audio_t *)handle;
    switch (whence) {
    case SEEK_CUR:
        ptr->curptr += offset;
        break;
    case SEEK_END:
        ptr->curptr = ptr->fileptr + ptr->fsize - offset;
        break;
    case SEEK_SET:
        ptr->curptr = ptr->fileptr + offset;
        break;
    default:
        return -1;
    };

    if (ptr->curptr < ptr->fileptr) {
        ptr->curptr = ptr->fileptr;
        return -1;
    }
    if (ptr->curptr > ptr->fileptr + ptr->fsize) {
        ptr->curptr = ptr->fileptr + ptr->fsize;
        return -1;
    }
    return 0;
}
int close_vio(void *handle)
{
    Z_Free(static_cast<bff_audio_t*>(handle)->filebuf);
    return 0;
}
long tell_vio(void *handle)
{
    bff_audio_t *ptr = (bff_audio_t*)handle;
    return (ptr->curptr - ptr->fileptr);
}
sf_count_t read_vio(void *dst, sf_count_t count, void *handle)
{
    bff_audio_t *ptr = (bff_audio_t*)handle;
    if ((ptr->curptr + count) > (ptr->fileptr + ptr->fsize))
        count = (ptr->fileptr + ptr->fsize) - ptr->curptr;
    
    memcpy(dst, ptr->curptr, count);
    ptr->curptr += count;
    return count;
}
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
        ptr->buffer.resize(ptr->fsize / sizeof(int16_t));
        fread(ptr->buffer.data(), sizeof(int16_t), ptr->buffer.size(), bff->fp);
    
#if 0
        ptr->curptr = ptr->filebuf;
        ptr->fileptr = ptr->filebuf;
        SF_VIRTUAL_IO vf;
        vf.get_filelen = filesize_vio;
        vf.read = read_vio;
        vf.seek = seek_vio;
        vf.tell = tell_vio;
        vf.write = NULL;
        SF_INFO fdata;
        SNDFILE* sf = sf_open_virtual(&vf, SFM_READ, &fdata, (void *)ptr);
        if (!sf) {
            scf::audio::music_on = false;
            scf::audio::sfx_on = false;
            fclose(bff->fp);
            N_Error("G_LoadBFF: failed to load audio chunk %s, sndfile error: %s", ptr->name, sf_strerror(sf));
        }
        ptr->buffer = (int16_t *)Z_Malloc(sizeof(int16_t) * (fdata.frames * fdata.channels), TAG_CACHE, &ptr->buffer);
        sf_count_t read = sf_readf_short(sf, (short *)ptr->buffer, fdata.frames);
        sf_close(sf);
#endif
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

        sfx_cache = (nomadsnd_t *)Z_Realloc(sfx_cache, sizeof(nomadsnd_t) * (i ? i : 1), &sfx_cache, TAG_CACHE);
        alGenSources(1, &sfx_cache[i].source);
        alGenBuffers(1, &sfx_cache[i].buffer);
        alBufferData(sfx_cache[i].buffer, ptr->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            ptr->buffer.data(), ptr->buffer.size() * sizeof(int16_t), ptr->samplerate);
        alSourcei(sfx_cache[i].source, AL_BUFFER, sfx_cache[i].buffer);
        alSourcef(sfx_cache[i].source, AL_GAIN, scf::audio::sfx_vol);
        ptr->buffer.clear();
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
            fdata.format = SF_FORMAT_VORBIS | SF_FORMAT_OGG;

            SNDFILE* sf = sf_open(std::string(std::string(dirname)+sndfile).c_str(), SFM_READ, &fdata);
            if (!sf) {
                N_Error("G_WriteBFF: failed to open audio file %s", std::string(std::string(dirname)+sndfile).c_str());
            }
            size_t read;
            ptr->fsize = 0;
            int16_t buffer[4096];
            while ((read = sf_read_short(sf, buffer, sizeof(buffer))) != 0) {
                ptr->buffer.insert(ptr->buffer.end(), buffer, buffer + read);
            }
            ptr->fsize = ptr->buffer.size() * sizeof(int16_t);
            
            ptr->samplerate = fdata.samplerate;
            ptr->channels = fdata.channels;
            sf_close(sf);
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
        bff->sounds[i].buffer.clear();
    }
    fclose(fp);

    exit(EXIT_SUCCESS);
}