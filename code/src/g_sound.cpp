#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include "n_shared.h"
#include "g_bff.h"
#include "g_game.h"
#include "n_scf.h"
#include "g_sound.h"
#include <sndfile.h>
#include "stb_vorbis.c"

static ALCdevice* device;
static ALCcontext* context;

uint32_t sndcache_size;
nomadsnd_t* snd_cache;

#define MAX_SND_HASH 1024
static nomadsnd_t *sndhash[MAX_SND_HASH];

static void Snd_LoadOGG(nomadsnd_t* snd, bffsound_t* buffer)
{
    char temppath[256];
    stbsp_sprintf(temppath, "gamedata/sndfile.ogg");
    FILE *fp = fopen(temppath, "wb");
    if (!fp) {
        N_Error("Snd_LoadOGG: failed to create temporary audio file");
    }
    fwrite(buffer->fileBuffer, 1, buffer->fileSize, fp);
    fclose(fp);

    snd->length = stb_vorbis_decode_filename(temppath, &snd->channels, &snd->samplerate, &snd->sndbuf);
    if (snd->length == -1) {
        N_Error("Snd_LoadOGG: stb_vorbis_decode_filename failed");
    }

    remove(temppath);
}

void I_CacheAudio(void *bffinfo)
{
    const bffinfo_t *info = (bffinfo_t *)bffinfo;
	FILE* fp;
	SNDFILE* sf;
	SF_INFO fdata;
	sf_count_t readcount;
	float sfx_vol = atof(snd_sfxvol.value);
	float music_vol = atof(snd_musicvol.value);

    snd_cache = (nomadsnd_t *)Hunk_Alloc(sizeof(nomadsnd_t) * info->numSounds, "snd_cache", h_low);
    sndcache_size = info->numSounds;

    for (uint32_t i = 0; i < info->numSounds; i++) {
        fp = tmpfile();
        if (!fp) {
            N_Error("I_CacheAudio: failed to create temporary audio file");
        }
        fwrite(info->sounds[i].fileBuffer, 1, info->sounds[i].fileSize, fp);
        fseek(fp, 0L, SEEK_SET);

        sf = sf_open_fd(fileno(fp), SFM_READ, &fdata, SF_TRUE);
        if (!sf) {
            N_Error("I_CacheAudio: failed to create temporary audio stream for chunk %s, libsndfile error: %s",
                info->sounds[i].name, sf_strerror(sf));
        }

        snd_cache[i].sndbuf = (short *)Hunk_TempAlloc(sizeof(short) * fdata.frames * fdata.channels);
        readcount = sf_read_short(sf, snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels);
        if (!readcount) {
            N_Error("I_CacheAudio: libsndfile failed to decode audio chunk %s, libsndfile error: %s",
                info->sounds[i].name, sf_strerror(sf));
        }
        sf_close(sf);

        snd_cache[i].samplerate = fdata.samplerate;
        snd_cache[i].channels = fdata.channels;
        snd_cache[i].length = fdata.samplerate * fdata.frames;

        alGenSources(1, &snd_cache[i].source);
        alGenBuffers(1, &snd_cache[i].buffer);
        alBufferData(snd_cache[i].buffer, snd_cache[i].channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels, snd_cache[i].samplerate);
        alSourcei(snd_cache[i].source, AL_BUFFER, snd_cache[i].buffer);
		alSourcef(snd_cache[i].source, AL_GAIN, music_vol);

        sndhash[Com_GenerateHashValue(info->sounds[i].name, MAX_SND_HASH)] = &snd_cache[i];
        snd_cache[i].failed = false;
    }
}

void Snd_Kill()
{
    Con_Printf("Snd_Kill: deallocating and freeing OpenAL sources and buffers");
    if ((!device || !context) || !snd_cache)
        return;
    
    for (uint32_t i = 0; i < numsfx; ++i) {
        if (!snd_cache[i].failed) {
            alSourcei(snd_cache[i].source, AL_BUFFER, 0);
            alDeleteBuffers(1, &snd_cache[i].buffer);
            alDeleteSources(1, &snd_cache[i].source);
        }
    }
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void P_PlaySFX(uint32_t sfx)
{
    assert(sfx < numsfx);
    snd_cache[sfx].queued = true;
}

nomadsnd_t* Snd_FetchSnd(const char *name)
{
    return sndhash[Com_GenerateHashValue(name, MAX_SND_HASH)];
}

void Snd_PlayTrack(const char *name)
{
    nomadsnd_t *snd = Snd_FetchSnd(name);
    if (!snd) {
        N_Error("Snd_PlayTrack: no such track named '%s'", name);
    }

    snd->queued = true;
}

void G_RunSound()
{
    for (uint32_t i = 0; i < sndcache_size; i++) {
        if (snd_cache[i].queued && !snd_cache[i].failed) {
            alSourcePlay(snd_cache[i].source);
            snd_cache[i].queued = false;
        }
    }
}

void Snd_Init()
{
    if (!N_strtobool(snd_sfxon.value) || !N_strtobool(snd_musicon.value))
        return;

    Con_Printf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        N_strncpy(snd_sfxon.value, "false", 5);
        N_strncpy(snd_musicon.value, "false", 5);
        Con_Printf("Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }
    assert(device);

    context = alcCreateContext(device, NULL);
    if (!context) {
        N_strncpy(snd_sfxon.value, "false", 5);
        N_strncpy(snd_musicon.value, "false", 5);
        Con_Printf("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    assert(context);
    alcMakeContextCurrent(context);

    Con_Printf("Snd_Init: successfully initialized sound libraries");
}