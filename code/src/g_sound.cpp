#include "n_shared.h"
#include "g_bff.h"
#include "g_game.h"
#include "n_scf.h"
#include "g_sound.h"

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <sndfile.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <pulse/pulseaudio.h>
#include "stb_vorbis.c"

static ALCdevice* device;
static ALCcontext* context;

cvar_t *snd_musicvol;
cvar_t *snd_sfxvol;
cvar_t *snd_musicon;
cvar_t *snd_sfxon;

typedef struct nomadsnd_s
{
    uint32_t source = 0;
    uint32_t buffer = 0;
    char name[80];

    int samplerate;
    int channels;
    int length;
    int frames;
    short* sndbuf;

    bool queued = false;
    bool failed = true; // if the pre-caching effort failed for this specific sound
} nomadsnd_t;

static uint64_t sndcache_size;
static nomadsnd_t* snd_cache;
static boost::mutex sndLock;

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

void I_CacheAudio(void)
{
    uint64_t numchunks;
    char **chunklist;
    FILE *fp;
    uint64_t bufferlen;
    char *buffer;
	SNDFILE* sf;
	SF_INFO fdata;
	sf_count_t readcount;

    Con_Printf("I_CacheAudio: allocating OpenAL buffers and sources");

    chunklist = FS_GetCurrentChunkList(&numchunks);
    sndcache_size = 0;

    for (uint64_t i = 0; i < numchunks; i++) {
        if (strstr(chunklist[i], ".ogg") != NULL || strstr(chunklist[i], ".wav") != NULL)
            sndcache_size++;
    }
    snd_cache = (nomadsnd_t *)Z_Malloc(sizeof(nomadsnd_t) * sndcache_size, TAG_SFX, &snd_cache, "snd_cache");
    for (uint64_t i = 0; i < numchunks; i++) {
        if (strstr(chunklist[i], ".ogg") == NULL && strstr(chunklist[i], ".wav") == NULL)
            continue;
        
        memset(&fdata, 0, sizeof(fdata));
        readcount = 0;

        bufferlen = FS_LoadFile(chunklist[i], (void **)&buffer);
        if (!buffer) {
            N_Error("I_CacheAudio: failed to open audio file %s", chunklist[i]);
        }

        fp = tmpfile();
        if (!fp) {
            N_Error("I_CacheAudio: failed to create temporary audio file");
        }

        fwrite(buffer, bufferlen, 1, fp);
        fseek(fp, 0L, SEEK_SET);

        sf = sf_open_fd(fileno(fp), SFM_READ, &fdata, SF_TRUE);
        if (!sf) {
            N_Error("I_CacheAudio: failed to create temporary audio stream for chunk %s, libsndfile error: %s",
                chunklist[i], sf_strerror(sf));
        }

        snd_cache[i].sndbuf = (short *)Z_Malloc(sizeof(short) * fdata.frames * fdata.channels, TAG_STATIC, &snd_cache[i].sndbuf, "sndbuf");
        readcount = sf_read_short(sf, snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels);
        if (!readcount) {
            N_Error("I_CacheAudio: libsndfile failed to decode audio chunk %s, libsndfile error: %s",
                chunklist[i], sf_strerror(sf));
        }
        sf_close(sf);

        snd_cache[i].samplerate = fdata.samplerate;
        snd_cache[i].channels = fdata.channels;
        snd_cache[i].length = fdata.samplerate * fdata.frames;
        snd_cache[i].frames = fdata.frames;

        N_strncpyz(snd_cache[i].name, chunklist[i], MAX_BFF_CHUNKNAME);
        alGenSources(1, (ALuint *)&snd_cache[i].source);
        alGenBuffers(1, (ALuint *)&snd_cache[i].buffer);
        alBufferData(snd_cache[i].buffer, snd_cache[i].channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            snd_cache[i].sndbuf, sizeof(short) * fdata.frames * fdata.channels, snd_cache[i].samplerate);
        alSourcei(snd_cache[i].source, AL_BUFFER, snd_cache[i].buffer);
		alSourcef(snd_cache[i].source, AL_GAIN, snd_musicvol->f);

        sndhash[Com_GenerateHashValue(chunklist[i], MAX_SND_HASH)] = &snd_cache[i];
        snd_cache[i].failed = false;
    }
}

void Snd_Shutdown(void)
{
    Con_Printf("Snd_Shutdown: deallocating and freeing OpenAL sources and buffers");
    if ((!device || !context) || !snd_cache)
        return;
    
    for (uint32_t i = 0; i < numsfx; ++i) {
        if (!snd_cache[i].failed) {
            if (snd_cache[i].source)
               alSourcei(snd_cache[i].source, AL_BUFFER, 0);
            if (snd_cache[i].buffer)
                alDeleteBuffers(1, (const ALuint *)&snd_cache[i].buffer);
            if (snd_cache[i].source)
                alDeleteSources(1, (const ALuint *)&snd_cache[i].source);
        }
    }
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void P_PlaySFX(uint32_t sfx)
{
    boost::unique_lock<boost::mutex> lock{sndLock};
    assert(sfx < numsfx);
    snd_cache[sfx].queued = true;
}

nomadsnd_t* Snd_FetchSnd(const char *name)
{
    return sndhash[Com_GenerateHashValue(name, MAX_SND_HASH)];
}

void Snd_PlayTrack(const char *name)
{
    boost::unique_lock<boost::mutex> lock{sndLock};
    nomadsnd_t *snd = Snd_FetchSnd(name);
    if (!snd) {
        N_Error("Snd_PlayTrack: no such track named '%s'", name);
    }

    snd->queued = true;
}

static void Snd_MusicInfo_f(void)
{
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
        }
    }
}

void Snd_Init(void)
{
    if (!snd_sfxon->i || !snd_musicon->i)
        return;

    Con_Printf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        snd_sfxon = Cvar_Get("snd_sfxon", "0", CVAR_PRIVATE | CVAR_SAVE);
        snd_musicon = Cvar_Get("snd_musicon", "0", CVAR_PRIVATE | CVAR_SAVE);
        Con_Printf("Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }
    assert(device);

    context = alcCreateContext(device, NULL);
    if (!context) {
        snd_sfxon = Cvar_Get("snd_sfxon", "0", CVAR_PRIVATE | CVAR_SAVE);
        snd_musicon = Cvar_Get("snd_musicon", "0", CVAR_PRIVATE | CVAR_SAVE);
        Con_Printf("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    snd_sfxon = Cvar_Get("snd_sfxon", "1", CVAR_PRIVATE | CVAR_SAVE);
    snd_musicon = Cvar_Get("snd_musicon", "1", CVAR_PRIVATE | CVAR_SAVE);
    assert(context);
    alcMakeContextCurrent(context);

    Cmd_AddCommand("musicinfo", Snd_MusicInfo_f);
    Cmd_AddCommand("sndinfo", Snd_DriverInfo_f);

    Con_Printf("Snd_Init: successfully initialized sound libraries");
}