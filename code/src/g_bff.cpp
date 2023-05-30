#include "n_shared.h"
#include "../bff_file/g_bff.h"
#include "g_bff.h"
#include "../common/n_vm.h"
#include "g_game.h"
#include "g_sound.h"
#include "m_renderer.h"
#include <sndfile.h>

uint32_t sndcache_size;
nomadsnd_t* snd_cache;
static bffinfo_t* bffinfo;

static void I_CacheAudio(bffinfo_t *info)
{
    FILE *fp;
    snd_cache = (nomadsnd_t *)Hunk_Alloc(sizeof(nomadsnd_t) * info->numSounds, "snd_cache", h_low);
    sndcache_size = info->numSounds;

    for (uint32_t i = 0; i < info->numSounds; i++) {
        // this isn't really ideal
        fp = tmpfile();
        if (!fp) {
            N_Error("I_CacheAudio: failed to create a temporary file");
        }
        fwrite(info->sounds[i].fileBuffer, info->sounds[i].fileSize, 1, fp);
        fseek(fp, 0L, SEEK_SET);
        
        SF_INFO fdata;
        SNDFILE* sf = sf_open_fd(fileno(fp), SFM_READ, &fdata, SF_TRUE);

        snd_cache[i].channels = fdata.channels;
        snd_cache[i].samplerate = fdata.samplerate;
        snd_cache[i].length = fdata.channels * fdata.frames;
        snd_cache[i].sndbuf = (short *)Z_Malloc(sizeof(short) * snd_cache[i].length, TAG_STATIC, &snd_cache[i].sndbuf, "sndbuffer");
        
        sf_count_t read = sf_read_short(sf, snd_cache[i].sndbuf, snd_cache[i].length);
        if (read != snd_cache[i].length) {
            N_Error("I_CacheAudio: short read from sf_read_short");
        }
        sf_close(sf);

        alGenSources(1, &snd_cache[i].source);
        alGenBuffers(1, &snd_cache[i].buffer);
        alBufferData(snd_cache[i].buffer, snd_cache[i].channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            snd_cache[i].sndbuf, snd_cache[i].length, snd_cache[i].samplerate);
        alSourcei(snd_cache[i].source, AL_BUFFER, snd_cache[i].buffer);

        Z_ChangeTag(snd_cache[i].sndbuf, TAG_CACHE);
    }
}

void B_LoadChunk(bff_chunk_t *chunk, FILE* fp)
{

}

bff_t* B_AddModule(const char *bffpath)
{
    bff_t* archive;
    bffheader_t header;

    FILE* fp = Sys_FOpen(bffpath, "rb");
    if (!fp) {
        N_Error("B_AddModule: failed to open bff %s", bffpath);
    }
    fread(&header, sizeof(bffheader_t), 1, fp);
    if (header.ident != BFF_IDENT) {
        N_Error("B_AddModule: bad archive, invalid identifier");
    }
    if (header.magic != HEADER_MAGIC) {
        N_Error("B_AddModule: bad archive, incorrect header magic");
    }
    if (!header.numChunks) {
        N_Error("B_AddModule: bad archive, bad chunk count");
    }

    archive = (bff_t *)Hunk_Alloc(sizeof(bff_t), "bffArchive", h_low);
    archive->numChunks = header.numChunks;
    archive->chunkList = (bff_chunk_t *)Hunk_Alloc(sizeof(bff_chunk_t) * archive->numChunks, "bffChunks", h_low);
    memset(archive->chunkList, 0, sizeof(bff_chunk_t) * header.numChunks);
    for (uint32_t i = 0; i < header.numChunks; i++) {
        B_LoadChunk(&archive->chunkList[i], fp);
    }
    fclose(fp);

    return archive;
}

void G_LoadBFF(const std::string& bffname)
{
    bff_t* archive = BFF_OpenArchive(bffname);
    bffinfo = BFF_GetInfo(archive);

    I_CacheAudio(bffinfo);

    Game::Init();

//    VM_Init(bffinfo->scripts);
}