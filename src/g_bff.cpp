#include "n_shared.h"
#include "g_game.h"

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

#include <sndfile.h>

static void B_CacheAudio(bff_audio_t* sounds, uint16_t numsounds)
{
	for (uint16_t i = 0; i < numsounds; ++i) {
		fread(sounds[i].name, sizeof(char), 80, bff->fp);
		fread(&sounds[i].lvl_index, sizeof(int32_t), 1, bff->fp);
		fread(&sounds[i].fsize, sizeof(uint64_t), 1, bff->fp);
        fread(&sounds[i].channels, sizeof(int32_t), 1, bff->fp);
        fread(&sounds[i].samplerate, sizeof(int32_t), 1, bff->fp);
        fread(&sounds[i].is_sfx, sizeof(bool), 1, bff->fp);
		
		sounds[i].buffer = (int16_t *)Z_Malloc(sounds[i].fsize, TAG_CACHE, &sounds[i].buffer);
		
		fread(sounds[i].buffer, sizeof(int16_t), sounds[i].fsize, bff->fp);
	}
}
static void B_CacheSpawns(bff_spawn_t* spawns, uint16_t numspawns)
{
	for (uint16_t i = 0; i < numspawns; ++i) {
		fread(spawns[i].name, sizeof(char), 80, bff->fp);
		fread(&spawns[i].what, sizeof(uint8_t), 1, bff->fp);
		fread(&spawns[i].marker, sizeof(uint16_t), 1, bff->fp);
        fread(&spawns[i].replacement, sizeof(uint16_t), 1, bff->fp);
		
		switch (spawns[i].what) {
		case ET_MOB:
			spawns[i].heap = Z_Malloc(sizeof(Mob), TAG_CACHE, &spawns[i].heap);
			break;
		case ET_PLAYR:
			spawns[i].heap = Z_Malloc(sizeof(playr_t), TAG_CACHE, &spawns[i].heap);
			break;
		case ET_ITEM:
			spawns[i].heap = Z_Malloc(sizeof(item_t), TAG_CACHE, &spawns[i].heap);
			break;
		case ET_WEAPON:
			spawns[i].heap = Z_Malloc(sizeof(weapon_t), TAG_CACHE, &spawns[i].heap);
			break;
		};
	}
}
static void B_CacheLevels(bff_level_t* levels, uint16_t numlevels)
{
	for (uint16_t i = 0; i < numlevels; ++i) {
		fread(levels[i].name, sizeof(char), 80, bff->fp);
		for (uint8_t s = 0; s < NUMSECTORS; ++s) {
			fread(levels[i].lvl_map[s], sizeof(uint16_t), SECTOR_MAX_Y * SECTOR_MAX_X, bff->fp);
		}
		
		// link
		uint16_t search;
		uint16_t snd_index = 0;
        for (search = 0; search < bff->header.numspawns; ++search) {
            for (uint8_t m = 0; m < NUMSECTORS; ++m) {
                for (uint16_t y = 0; y < SECTOR_MAX_Y; ++y) {
                    for (uint16_t x = 0; x < SECTOR_MAX_X; ++x) {
                        if (levels[i].lvl_map[m][y][x] == bff->spawns[search].marker) {
                            levels[i].lvl_map[m][y][x] = bff->spawns[search].replacement;
                            bff->spawns[search].where = {y, x};
                            ++levels[i].numspawns;
                        }
                    }
                }
            }
        }
		for (search = 0; search < bff->header.numsounds; ++search) {
			if (bff->sounds[search].lvl_index != -1 && bff->sounds[search].level == &levels[i]) {
				levels[i].tracks = (bff_audio_t *)Z_Realloc(levels[i].tracks,
					sizeof(bff_audio_t) * (snd_index == 0 ? 1 : snd_index), &levels[i].tracks);
				
				N_memmove(&levels[i].tracks[snd_index], &bff->sounds[search], sizeof(bff_audio_t));
				++snd_index;
			}
		}
		levels[i].numtracks = snd_index;
	}
}

void G_LoadBFF(const char* filename)
{
	bff = (bff_file_t *)Z_Malloc(sizeof(bff_file_t), TAG_STATIC, &bff);
	
	bff->fp = fopen(filename, "rb");
	if (!bff->fp) {
		N_Error("BFF_Init: failed to open file %s", filename);
	}
	
	fread(&bff->header, sizeof(bffinfo_t), 1, bff->fp);
	if (bff->header.magic != HEADER_MAGIC) {
		N_Error("BFF_Init: header wasn't the correct constant, should be %lu, got %lu",
			(uint64_t)HEADER_MAGIC, bff->header.magic);
	}
	
	bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_CACHE, &bff->levels);
	bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numsounds, TAG_CACHE, &bff->sounds);
	bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_CACHE, &bff->spawns);
	
	B_CacheAudio(bff->sounds, bff->header.numsounds);
	B_CacheSpawns(bff->spawns, bff->header.numspawns);
	B_CacheLevels(bff->levels, bff->header.numlevels);
	
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
        const std::string name = header["bffname"];
        N_strncpy(bff->header.name, name.c_str(), 80);
    }

    // load the spawns
    {
        bff->spawns = (bff_spawn_t *)Z_Malloc(sizeof(bff_spawn_t) * bff->header.numspawns, TAG_STATIC, &bff->spawns);

        for (uint16_t i = 0; i < bff->header.numspawns; ++i) {
            const std::string node_name = "spawner_"+std::to_string(i);
            const json spawn = data[node_name];
            bff_spawn_t* ptr = &bff->spawns[i];

            ptr->replacement = spawn["replacement"];
            ptr->marker = spawn["marker"];
            const std::string type = spawn["type"];
            if (type == "ET_MOB")
                ptr->what = ET_MOB;
            else if (type == "ET_PLAYR")
                ptr->what = ET_PLAYR;
            else if (type == "ET_ITEM")
                ptr->what = ET_ITEM;
            else if (type == "ET_WEAPON")
                ptr->what = ET_WEAPON;
            
            const std::string name = spawn["name"];
            N_strncpy(ptr->name, name.c_str(), 80);
        }
    }

    // load the levels
    {
        bff->levels = (bff_level_t *)Z_Malloc(sizeof(bff_level_t) * bff->header.numlevels, TAG_STATIC, &bff->levels);

        for (uint16_t i = 0; i < bff->header.numlevels; ++i) {
            const std::string node_name = "level_"+std::to_string(i);
            const json lvl = data[node_name];
            bff_level_t* ptr = &bff->levels[i];

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
                        ptr->lvl_map[m][y][x] = (uint16_t)spr;
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
    // load the sounds
    {
        bff->sounds = (bff_audio_t *)Z_Malloc(sizeof(bff_audio_t) * bff->header.numsounds, TAG_STATIC, &bff->sounds);
        
        for (uint16_t i = 0; i < bff->header.numsounds; ++i) {
            const std::string node_name = "sound_"+std::to_string(i);
            const json snd = data[node_name];
            bff_audio_t* ptr = &bff->sounds[i];

            const std::string sndfile = snd["filepath"];

            // store the uncompressed sound data
            SF_INFO fdata;
            SNDFILE* sf = sf_open(std::string(std::string(dirname)+sndfile).c_str(), SFM_READ, &fdata);
            if (!sf) {
                N_Error("G_WriteBFF: failed to open audio file %s", std::string(std::string(dirname)+sndfile).c_str());
            }
            size_t read;
            int16_t buffer[4096];
            N_memset(buffer, 0, sizeof(buffer));
            std::vector<int16_t> rdbuf;
            while ((read = sf_read_short(sf, buffer, sizeof(buffer))) != 0) {
                rdbuf.insert(rdbuf.end(), buffer, buffer + read);
            }
            ptr->numbuffers = rdbuf.size();
            ptr->fsize = rdbuf.size() * sizeof(int16_t);
            ptr->buffer = (int16_t *)Z_Malloc(ptr->fsize, TAG_STATIC, &ptr->buffer);
            N_memset(ptr->buffer, 0, ptr->fsize);
            N_memmove(ptr->buffer, rdbuf.data(), ptr->fsize);
        }
    }


    // write everything
    FILE* fp = fopen(outfile, "wb");
    if (!fp) {
        N_Error("G_WriteBFF: failed to open output bff file %s", outfile);
    }
    uint16_t i;

    fwrite(&bff->header, sizeof(bffinfo_t), 1, fp);

    for (i = 0; i < bff->header.numlevels; ++i) {
        fwrite(bff->levels[i].name, sizeof(char), 80, fp);
        for (uint8_t m = 0; m < NUMSECTORS; ++m) {
            fwrite(bff->levels[i].lvl_map[m], sizeof(uint16_t), sizeof(bff->levels[i].lvl_map[m]), fp);
        }
    }
    for (i = 0; i < bff->header.numspawns; ++i) {
        fwrite(bff->spawns[i].name, sizeof(char), 80, fp);
        fwrite(&bff->spawns[i].what, sizeof(uint8_t), 1, fp);
        fwrite(&bff->spawns[i].marker, sizeof(uint16_t), 1, fp);
        fwrite(&bff->spawns[i].replacement, sizeof(uint16_t), 1, fp);
    }
    for (i = 0; i < bff->header.numsounds; ++i) {
        fwrite(bff->sounds[i].name, sizeof(char), 80, fp);
		fwrite(&bff->sounds[i].lvl_index, sizeof(int32_t), 1, fp);
		fwrite(&bff->sounds[i].fsize, sizeof(uint64_t), 1, fp);
        fwrite(&bff->sounds[i].channels, sizeof(int32_t), 1, fp);
        fwrite(&bff->sounds[i].samplerate, sizeof(int32_t), 1, fp);
        fwrite(bff->sounds[i].buffer, sizeof(int16_t), bff->sounds[i].numbuffers, fp);
    }
    fclose(fp);

    exit(EXIT_SUCCESS);
}