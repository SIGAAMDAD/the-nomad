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
static pthread_mutex_t snd_lock = PTHREAD_MUTEX_INITIALIZER;

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

        snd_cache[i].sndbuf = (short *)Mem_Alloc(sizeof(short) * fdata.frames * fdata.channels);
        readcount = sf_read_short(sf, snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels);
        if (!readcount) {
            N_Error("I_CacheAudio: libsndfile failed to decode audio chunk %s, libsndfile error: %s",
                info->sounds[i].name, sf_strerror(sf));
        }
        sf_close(sf);

        snd_cache[i].samplerate = fdata.samplerate;
        snd_cache[i].channels = fdata.channels;
        snd_cache[i].length = fdata.samplerate * fdata.frames;
        snd_cache[i].frames = fdata.frames;

        N_strncpy(snd_cache[i].name, info->sounds[i].name, MAX_BFF_CHUNKNAME);
        alGenSources(1, &snd_cache[i].source);
        alGenBuffers(1, &snd_cache[i].buffer);
        alBufferData(snd_cache[i].buffer, snd_cache[i].channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels, snd_cache[i].samplerate);
        alSourcei(snd_cache[i].source, AL_BUFFER, snd_cache[i].buffer);
		alSourcef(snd_cache[i].source, AL_GAIN, snd_musicvol.f);
        Mem_Free(snd_cache[i].sndbuf);

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
    pthread_mutex_lock(&snd_lock);
    nomadsnd_t *snd = Snd_FetchSnd(name);
    if (!snd) {
        N_Error("Snd_PlayTrack: no such track named '%s'", name);
    }

    snd->queued = true;
    pthread_mutex_unlock(&snd_lock);
}

static nomadsnd_t *track; // currently playing music chunk

static void Snd_MusicInfo_f(void)
{
    if (!track) {
        Con_Printf("No music track is currently playing");
    }

    Con_Printf("\n<---------- Music Info ---------->");
    Con_Printf("Track Name: %s", track->name);
    Con_Printf("Duration: %lu", (uint64_t)(track->frames / (float)track->samplerate));
}

static void Snd_DriverInfo_f(void)
{
    Con_Printf("\n<---------- OpenAL Info ---------->");
    Con_Printf("ALC_MAJOR_VERSION: %lu", ALC_MAJOR_VERSION);
    Con_Printf("ALC_MINOR_VERSION: %lu", ALC_MINOR_VERSION);
    Con_Printf("ALC_EXTENSIONS: %s", alcGetString(device, ALC_EXTENSIONS));
}

void Snd_Submit(void)
{
    for (uint32_t i = 0; i < sndcache_size; ++i) {
        if (snd_cache[i].queued) {
            alSourcePlay(snd_cache[i].source);
            snd_cache[i].queued = false;
            if (strstr(snd_cache[i].name, "MUS")) { // music chunk
                track = &snd_cache[i];
            }
        }
    }
}

void Snd_Init()
{
    if (!snd_sfxon.b || !snd_musicon.b)
        return;

    Con_Printf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        snd_sfxon.b = qfalse;
        snd_musicon.b = qfalse;
        Con_Printf("Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }
    assert(device);

    context = alcCreateContext(device, NULL);
    if (!context) {
        snd_sfxon.b = qfalse;
        snd_musicon.b = qfalse;
        Con_Printf("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    snd_sfxon.b = qtrue;
    snd_musicon.b = qtrue;
    assert(context);
    alcMakeContextCurrent(context);

    Cmd_AddCommand("musicinfo", Snd_MusicInfo_f);
    Cmd_AddCommand("sndinfo", Snd_DriverInfo_f);

    Con_Printf("Snd_Init: successfully initialized sound libraries");
}