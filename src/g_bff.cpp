#include "n_shared.h"
#include "g_game.h"
#include "stb_vorbis.c"

#ifdef _WIN32
#include "dirent_win32.h"
#endif

#define FILEPATH(x,ext,bff) std::string(std::string("Files/gamedata/BFF/")+bff+"/"+std::string(x)+ext).c_str()

bff_file_t* bff;
bff_level_t* levels;
bff_texture_t* textures;
bff_spawn_t* spawns;
bffinfo_t bffinfo;

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
    static char buffer[BFF_STR_SIZE + 1];
    fread(buffer, sizeof(char), BFF_STR_SIZE + 1, bff->fp);
    return buffer;
}

uint64_t extra_heap;

int remove_recursive(const char* path)
{
    DIR* directory = opendir(path);
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory))) {
            if (!strcmp(".", entry->d_name) || !strcmp("..", entry->d_name))
                continue;
            
            char filename[strlen(path) + strlen(entry->d_name) + 2];
            stbsp_sprintf(filename, "%s/%s", path, entry->d_name);
            int (*remove_func)(const char *) = entry->d_type == DT_DIR ? remove_recursive : remove;
            if (remove_func(entry->d_name)) {
                closedir(directory);
                return -1;
            }
        }
        if (closedir(directory))
            return -1;
    }
    return remove(path);
}

void G_ExtractBFF(const std::string& filepath)
{
    Z_Init();
    bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff, "BFF");

	bff->fp = fopen(filepath.c_str(), "rb");
	if (!bff->fp) {
		N_Error("G_ExtractBFF: failed to open file %s", filepath.c_str());
	}
	
    memset(&bff->header, 0, sizeof(bffinfo_t));
	fread(&bff->header, sizeof(bffinfo_t), 1, bff->fp);
	if (bff->header.magic != HEADER_MAGIC) {
		N_Error("G_ExtractBFF: header wasn't the correct constant, should be %lx, got %lx",
			(uint64_t)HEADER_MAGIC, bff->header.magic);
	}

    LOG_INFO("number of level chunks to load: {}", bff->header.numlevels);
    LOG_INFO("number of spawn chunks to load: {}", bff->header.numspawns);
    LOG_INFO("number of sound chunks to load: {}", bff->header.numsounds);
    LOG_INFO("number of texture chunks to load: {}", bff->header.numtextures);
	
	bff->levels = bff->header.numlevels
        ? (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_STATIC, &bff->levels, "bfflvls") : NULL;
	bff->sounds = bff->header.numsounds
        ? (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_STATIC, &bff->sounds, "bffsnds") : NULL;
	bff->spawns = bff->header.numspawns
        ? (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_STATIC, &bff->spawns, "bffspns") : NULL;
    bff->textures = bff->header.numtextures
        ? (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_STATIC, &bff->textures, "bfftextures") : NULL;


    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        bff_level_t* const ptr = &bff->levels[i];
        memset(ptr, 0, sizeof(bff_level_t));

        LOG_INFO("loading level chunk {}", i);
        fread(&ptr->spawncount, sizeof(uint16_t), 1, bff->fp);
        if (ptr->spawncount) {
            ptr->spawnlist = (uint16_t *)Z_Malloc(sizeof(uint16_t) * ptr->spawncount, TAG_STATIC, &ptr->spawnlist, "spnlist");
            fread(ptr->spawnlist, sizeof(uint16_t), ptr->spawncount, bff->fp);
        }
        fread(ptr->lvl_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X, bff->fp);
#if 0
        char *inbuffer;
        unsigned int insize;
        unsigned int destlen = sizeof(ptr->lvl_map);

        fread(&insize, sizeof(unsigned int), 1, bff->fp);
        inbuffer = (char *)malloc(insize);
        if (!inbuffer) {
            fclose(bff->fp);
            N_Error("G_LoadBFF: malloc() failed");
        }
        fread(inbuffer, sizeof(char), insize, bff->fp);
        int ret = BZ2_bzBuffToBuffDecompress((char *)ptr, &destlen, inbuffer, insize, 0, 0);
        if (ret != BZ_OK) {
            free(inbuffer);
            fclose(bff->fp);
            N_Error("G_LoadBFF: failed to decompress %i bytes using bzip2, error: %s", insize, bzip2_strerror(ret));
        }
        LOG_TRACE("successfully decompressed {} bytes of level data", insize);
        free(inbuffer);
#endif
    }
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
        bff_spawn_t* const ptr = &bff->spawns[i];
        memset(ptr, 0, sizeof(bff_spawn_t));

        LOG_INFO("loading spawn chunk {}", i);
        fread(&ptr->what, sizeof(uint8_t), 1, bff->fp);
        fread(&ptr->replacement, sizeof(sprite_t), 1, bff->fp);
        fread(&ptr->marker, sizeof(sprite_t), 1, bff->fp);
    }
    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        bff_texture_t* const ptr = &bff->textures[i];
        memset(ptr, 0, sizeof(bff_texture_t));
        
        LOG_INFO("loading texture chunk {}", i);
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        ptr->buffer = (char *)Z_Malloc(ptr->fsize, TAG_STATIC, &ptr->buffer);
        fread(ptr->buffer, sizeof(char), ptr->fsize, bff->fp);
    }
    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        bff_audio_t* const ptr = &bff->sounds[i];
        memset(ptr, 0, sizeof(bff_audio_t));
        LOG_INFO("loading audio chunk {}", i);

        fread(&ptr->lvl_index, sizeof(int32_t), 1, bff->fp);
        fread(&ptr->fsize, sizeof(uint64_t), 1, bff->fp);
        if (ptr->fsize) {
            ptr->filebuf = (char *)Z_Malloc(ptr->fsize, TAG_STATIC, &ptr->filebuf, "sndfilebuf");
            fread(ptr->filebuf, sizeof(char), ptr->fsize, bff->fp);
        }
    }
	fclose(bff->fp);

    std::string outdir = "Files/gamedata/BFF/"+filepath;
#ifdef __unix__
    int ret = mkdir(outdir.c_str(), (mode_t)0777);
#elif defined(_WIN32)
    int ret = mkdir(outdir.c_str());
#endif
    switch (ret) {
    case EACCES:
        N_Error("G_ExtractBFF: failed to create bff directory because of invalid permissions, (EACCESS), perhaps run as admin/root?");
        break;
    case ENAMETOOLONG:
        N_Error("G_ExtractBFF: failed to create bff directory because the name was too longth, must be less than %i characters",
            PATH_MAX);
        break;
    case EROFS:
        N_Error("G_ExtractBFF: failed to create bff directory because the Files/gamedata/BFF is a read-only file system, perhaps change permissions?");
        break;
    case ELOOP:
        N_Error("G_ExtractBFF: failed to create bff directory because symbolic links were encountered in the file tree");
        break;
    default: break;
    };
    if (ret == EEXIST) {
        LOG_WARN("bff has already been extracted, overwrite? [y/n]");
        char in = getc(stdin);
        if (in == 'y') {
            remove_recursive(outdir.c_str());
            LOG_INFO("overwriting extracted bff file {}", filepath);
        }
        else {
            LOG_INFO("canceling extraction");
            exit(EXIT_SUCCESS);
        }
    }

    outdir += "/";
    LOG_INFO("output directory: {}", outdir);
    
    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        LOG_INFO("extracting level chunk {}", i);
        std::string path = "NMLVLFILE_"+std::to_string((int)i);
        N_WriteFile(FILEPATH(path, ".blf", filepath), &bff->levels[i], (MAP_MAX_Y * MAP_MAX_Y));
    }
    
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
        LOG_INFO("extracting spawn chunk {}", i);
        std::string path = "NMSPNFILE_"+std::to_string((int)i);
        N_WriteFile(FILEPATH(path, ".bsf", filepath), &bff->spawns[i], sizeof(bff_spawn_t) - sizeof(void *));
    }

    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        LOG_INFO("extracting texture chunk {}", i);
        std::string path = "NMTEXFILE_"+std::to_string((int)i);
        N_WriteFile(FILEPATH(path, ".bmp", filepath), bff->textures[i].buffer, bff->textures[i].fsize);
    }

    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        LOG_INFO("extracting audio chunk {}", i);
        std::string path = "NMSNDFILE_"+std::to_string((int)i);
        N_WriteFile(FILEPATH(path, ".ogg", filepath), bff->sounds[i].filebuf, bff->sounds[i].fsize);
    }

    FILE *fp = fopen(FILEPATH("bffinfo", ".dat", filepath), "wb");
    if (!fp) {
        N_Error("G_ExtractBFF: failed to create bff info file");
    }
    fwrite(&bff->header, sizeof(bffinfo_t), 1, fp);
    fclose(fp);
    
    exit(EXIT_SUCCESS);
}

void G_LoadBFF(const std::string& bffname)
{
    bffinfo_t *header;
    Z_Init();
    N_ReadFile(FILEPATH("bffinfo", ".dat", bffname), (char **)&header);

    std::vector<nomadsnd_t> sounds(header->numsounds);
    memset(sounds.data(), 0, sounds.size() * sizeof(nomadsnd_t));
    for (uint16_t i = 0; i < header->numsounds; ++i) {
        std::string filepath = "NMSNDFILE_"+std::to_string((int)i);
        int channels{};
        int samplerate{};
        short* buffer;
        int ret = stb_vorbis_decode_filename(FILEPATH(filepath, ".ogg", bffname), &channels, &samplerate, &buffer);
        
        alGenSources(1, &sounds[i].source);
        alGenBuffers(1, &sounds[i].buffer);
        alBufferData(sounds[i].buffer, channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            buffer, ret, samplerate);
        alSourcei(sounds[i].source, AL_BUFFER, sounds[i].buffer);
        xfree(buffer);
    }

    LOG_INFO("initiazing renderer");
    R_Init();

    Game::Init();

    // pre-cache the bff stuff
    memmove(&bffinfo, header, sizeof(bffinfo_t));
    xfree(header);

    levels = bffinfo.numlevels
        ? (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bffinfo.numlevels, TAG_STATIC, &levels, "bfflvls") : NULL;
	//bff->sounds = bff->header.numsounds
    //    ? (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_CACHE, &bff->sounds) : NULL;
	spawns = bffinfo.numspawns
        ? (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bffinfo.numspawns, TAG_STATIC, &spawns, "bffspns") : NULL;
    textures = bffinfo.numtextures
        ? (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bffinfo.numtextures, TAG_STATIC, &textures, "bfftextures") : NULL;

    for (uint16_t i = 0; i < bffinfo.numlevels; ++i) {
        std::string filepath = "NMLVLFILE_"+std::to_string(i);
        char *buffer;
        size_t size = N_ReadFile(FILEPATH(filepath, ".blf", bffname), &buffer);
        memcpy(&levels[i], buffer, sizeof(bff_level_t));
        xfree(buffer);
    }

    // transfer sound data from malloc to the zone
    sfx_cache = (nomadsnd_t *)Z_Malloc(sizeof(nomadsnd_t) * sounds.size(), TAG_STATIC, &sfx_cache);
    memcpy(sfx_cache, sounds.data(), sizeof(nomadsnd_t) * sounds.size());
    sounds.clear();

    // all mob/non-player-entity memory will be slapped onto the heap from this point forward,
    // and everything with TAG_CACHE or TAG_STATIC will remain allocated for the entirety of
    // runtime

    for (uint16_t i = 1; i < bffinfo.numspawns + 1; ++i) {
        Game::Get()->entities.emplace_back();
        memset(&Game::Get()->entities.back(), 0, sizeof(entity_t));
    }
    for (uint16_t i = 0; i < bffinfo.numsounds; ++i) {
        alGenSources(1, &sfx_cache[i].source);
        alGenBuffers(1, &sfx_cache[i].buffer);
        alSourcei(sfx_cache[i].source, AL_BUFFER, sfx_cache[i].buffer);
        alBufferData(sfx_cache[i].buffer, sfx_cache[i].channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            sfx_cache[i].sndbuf, sfx_cache[i].length * sizeof(short), sfx_cache[i].samplerate);
        alSourcef(sfx_cache[i].source, AL_GAIN, scf::audio::sfx_vol);
    }
    
    Game::Get()->playr->p = &Game::Get()->entities.front();

    memset(Game::Get()->playr->p, 0, sizeof(entity_t));
    memset(Game::Get()->playr->p->pos.hitbox, 0, sizeof(vec2_t) * 4);
    memset(&Game::Get()->playr->p->pos.thrust, 0, sizeof(vec3_t));
    memset(&Game::Get()->playr->p->pos.to, 0, sizeof(vec3_t));

    Z_Print(true);
    if (levels)
        Z_ChangeTag(levels, TAG_CACHE);
    if (spawns)
        Z_ChangeTag(spawns, TAG_CACHE);
    if (textures)
        Z_ChangeTag(textures, TAG_CACHE);
}

void G_WriteBFF(const char* outfile, const char* dirname)
{
    if (!outfile)
        return;

    Z_Init();
    bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff, "BFF");

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
    }

    // load the spawns
    if (bff->header.numspawns) {
        static auto char_to_sprite = [=](char c) -> sprite_t
        {
            switch (c) {
            case '#': return SPR_WALL; break;
            case '.': return SPR_FLOOR_INSIDE; break;
            case ' ': return SPR_FLOOR_OUTSIDE; break;
            case '_': return SPR_DOOR_STATIC; break;
            case '<': return SPR_DOOR_OPEN; break;
            case '>': return SPR_DOOR_CLOSE; break;
            case '&': return SPR_ROCK; break;
            case ';': return SPR_WATER; break;
            default: return SPR_CUSTOM; break;
            };
        };
        bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_STATIC, &bff->spawns, "bffspns");

        for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
            const std::string node_name = "spawner_"+std::to_string(i);
            bff_spawn_t* ptr = &bff->spawns[i];
            memset(ptr, 0, sizeof(*ptr));

            const std::string replacement = data[node_name]["replacement"];
            const std::string marker = data[node_name]["marker"];
            
            ptr->replacement = char_to_sprite(replacement[0]);
            ptr->marker = char_to_sprite(marker[0]);
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
            memset(ptr->entityid, 0, sizeof(ptr->entityid));
            strncpy(ptr->entityid, id.c_str(), 80);
        }
    }
    // load the levels
    if (bff->header.numlevels) {
        bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_STATIC, &bff->levels, "bfflvls");

        for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
            const std::string node_name = "level_"+std::to_string(i);
            const json lvl = data[node_name];
            bff_level_t* ptr = &bff->levels[i];
            memset(ptr, 0, sizeof(*ptr));

            LOG_INFO("loading level chunk {}", i);

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
                                ptr->spawnlist = (uint16_t *)Z_Realloc(ptr->spawnlist,
                                    sizeof(uint16_t) * (ptr->spawncount ? ptr->spawncount : 1), &ptr->spawnlist, TAG_STATIC, "spnlist");
                                ptr->spawnlist[ptr->spawncount] = s;
                                spn->where = {y, x};
                                ++ptr->spawncount;
                            }
                        }
                    }
                }
            }
        }
    }
    // load the textures
    if (bff->header.numtextures) {
        bff->textures = (bff_texture_t *)Z_Malloc(sizeof(bff_texture_t) * bff->header.numtextures, TAG_STATIC, &bff->textures, "bfftextures");

        for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
            const std::string node_name = "texture_"+std::to_string(i);
            const json tex = data[node_name];
            bff_texture_t* ptr = &bff->textures[i];
            memset(ptr, 0, sizeof(*ptr));
            const std::string texfile = tex["filepath"];
            ptr->fsize = N_ReadFile(std::string(std::string(dirname)+texfile).c_str(), &ptr->buffer);
        }
    }
    // load the sounds
    if (bff->header.numsounds) {
        bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_STATIC, &bff->sounds, "bffsnds");

        for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
            const std::string node_name = "sound_"+std::to_string(i);
            const json snd = data[node_name];
            bff_audio_t* ptr = &bff->sounds[i];
            memset(ptr, 0, sizeof(*ptr));

            const std::string sndfile = snd["filepath"];
            if (sndfile.find(".ogg") != std::string::npos || sndfile.find(".OGG") != std::string::npos)
                ptr->type = FT_OGG;
            else if (sndfile.find(".wav") != std::string::npos || sndfile.find(".WAV") != std::string::npos)
                ptr->type = FT_WAV;
            else if (sndfile.find(".flac") != std::string::npos || sndfile.find(".FLAC") != std::string::npos)
                ptr->type = FT_FLAC;
            else if (sndfile.find(".opus") != std::string::npos || sndfile.find(".OPUS") != std::string::npos)
                ptr->type = FT_OPUS;

            ptr->fsize = N_ReadFile(std::string(std::string(dirname)+sndfile).c_str(), &ptr->filebuf);
            LOG_INFO("done loading audio chunk {}", i);
        }
    }

    Z_Print(true);
    xalloc_stats();

    // write everything
    FILE* fp = fopen(outfile, "wb");
    if (!fp) {
        N_Error("G_WriteBFF: failed to open output bff file %s", outfile);
    }
    assert(fp);

    bff->header.magic = HEADER_MAGIC;
    fwrite(&bff->header, sizeof(bffinfo_t), 1, fp);

    LOG_INFO("number of level chunks to write: {}", bff->header.numlevels);
    LOG_INFO("number of spawn chunks to write: {}", bff->header.numspawns);
    LOG_INFO("number of sound chunks to write: {}", bff->header.numsounds);
    LOG_INFO("number of texture chunks to write: {}", bff->header.numtextures);

    for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
        LOG_INFO("writing level chunk {}", i);
        fwrite(&bff->levels[i].spawncount, sizeof(uint16_t), 1, fp);
        fwrite(bff->levels[i].spawnlist, sizeof(uint16_t), bff->levels[i].spawncount, fp);
        if (bff->levels[i].spawnlist) {
            Z_Free(bff->levels[i].spawnlist);
        }
        fwrite(bff->levels[i].lvl_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X, fp);
        fflush(fp);
    }
    for (uint16_t i = 0; i < bff->header.numspawns; ++i) { // no need to compress spawners
        fwrite(bff->spawns[i].entityid, sizeof(char), BFF_STR_SIZE + 1, fp);
        fwrite(&bff->spawns[i].what, sizeof(uint8_t), 1, fp);
        fwrite(&bff->spawns[i].replacement, sizeof(sprite_t), 1, fp);
        fwrite(&bff->spawns[i].marker, sizeof(sprite_t), 1, fp);
        fflush(fp);
    }
    for (uint16_t i = 0; i < bff->header.numtextures; ++i) {
        fwrite(&bff->textures[i].fsize, sizeof(uint64_t), 1, fp);
        fwrite(bff->textures[i].buffer, sizeof(char), bff->textures[i].fsize, fp);
        xfree(bff->textures[i].buffer);
        fflush(fp);
    }
    for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
        fwrite(&bff->sounds[i].type, sizeof(uint8_t), 1, fp);
        fwrite(&bff->sounds[i].lvl_index, sizeof(int32_t), 1, fp);
        fwrite(&bff->sounds[i].fsize, sizeof(uint64_t), 1, fp);
        fwrite(bff->sounds[i].filebuf, sizeof(char), bff->sounds[i].fsize, fp);
        xfree(bff->sounds[i].filebuf);
        LOG_INFO("wrote audio chunk {}", i);
        fflush(fp);
    }
    fclose(fp);
    exit(EXIT_SUCCESS);
}