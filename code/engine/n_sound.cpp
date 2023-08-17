#include "n_shared.h"
#include "n_scf.h"
#include "n_sound.h"

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <sndfile.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

static ALCdevice* device;
static ALCcontext* context;

static cvar_t *snd_musicvol;
static cvar_t *snd_sfxvol;
static cvar_t *snd_musicon;
static cvar_t *snd_sfxon;

#define Snd_HashFileName(x) Com_GenerateHashValue((x),MAX_SND_HASH)

typedef struct
{
    char name[MAX_GDR_PATH];

    uint32_t source;
    uint32_t buffer;
    uint32_t samplerate;
    uint32_t channels;
    uint32_t length;
    uint32_t frames;

    short *sndbuf;

    qboolean failed;
} nomadsnd_t;

#define SND_STATE_NOTPLAYING    0x0000
#define SND_STATE_PLAYING       0x2000

static uint64_t sndcache_size;
static nomadsnd_t* snd_cache;
static boost::shared_mutex sndLock;

#define MAX_SND_HASH 1024
static nomadsnd_t *sndhash[MAX_SND_HASH];
#define MAX_SND_QUEUE 256
static nomadsnd_t *sfxqueue[MAX_SND_QUEUE];
static uint32_t sfx_queuedCount;
static nomadsnd_t *musicTrack;

static uint32_t Snd_Format(const nomadsnd_t *snd)
{
    if (snd->channels == 1)
        return AL_FORMAT_MONO16;
    
    return AL_FORMAT_STEREO16;
}

/*
Snd_LoadAudio: loads a list of audio files into the audio cache
*/
static void Snd_LoadAudio(char **list, uint64_t numfiles)
{
    nomadsnd_t *snd;
    sf_count_t readCount;
    uint64_t fileLen, hash;
    char *buffer;
    SF_INFO fdata;
    SNDFILE *sf;
    FILE *fp;

    if (!numfiles) {
        return;
    }

    for (uint64_t i = 0; i < numfiles; i++) {
        snd = (nomadsnd_t *)Z_Malloc(sizeof(*snd), TAG_SFX, &snd, "snd");
        hash = Snd_HashFileName(list[i]);
        sndhash[hash] = snd;
        snd->failed = qtrue;

        fileLen = FS_LoadFile(list[i], (void **)&buffer);
        if (!buffer) {
            Con_Printf(ERROR, "Failed to load sound file %s", list[i]);
            continue;
        }

        fp = tmpfile();
        if (!fp) {
            N_Error("Snd_LoadAudio: tmpfile() failed");
        }
        fwrite(buffer, fileLen, 1, fp);
        fseek(fp, 0L, SEEK_SET);

        sf = sf_open_fd(fileno(fp), SFM_READ, &fdata, SF_TRUE);
        if (!sf) {
            N_Error("Snd_LoadAudio: sf_open_fd failed, error: %s", sf_strerror(sf));
        }

        // NOTE: perhaps use the hunk for this
        snd->sndbuf = (short *)Z_Malloc(sizeof(*snd->sndbuf) * snd->length, TAG_SFX, &snd->sndbuf, "sndbuf");
        readCount = sf_read_short(sf, snd->sndbuf, sizeof(*snd->sndbuf) * fdata.frames * fdata.channels);
        if (readCount != fileLen) {
            N_Error("Snd_LoadAudio: sf_read_short failed, error: %s", sf_strerror(sf));
        }
        sf_close(sf);

        alGenSources(1, (ALuint *)&snd->source);
        alGenBuffers(1, (ALuint *)&snd->buffer);
        alBufferData(snd->buffer, Snd_Format(snd), snd->sndbuf, sizeof(*snd->sndbuf) * snd->length, snd->samplerate);
        alSourcei(snd->source, AL_BUFFER, snd->buffer);

        snd->failed = qfalse;
    }
}

void I_CacheAudio(void)
{
    uint64_t OGGchunks, WAVchunks;
    char **ogglist, **wavlist;
    FILE *fp;
    uint64_t bufferlen;
    char *buffer;
	SNDFILE* sf;
	SF_INFO fdata;
	sf_count_t readcount;

    Con_Printf("I_CacheAudio: allocating OpenAL buffers and sources");

    ogglist = FS_ListFiles("", ".ogg", &OGGchunks);
    wavlist = FS_ListFiles("", ".wav", &WAVchunks);

    Snd_LoadAudio(ogglist, OGGchunks);
    Snd_LoadAudio(wavlist, WAVchunks);

    Sys_FreeFileList(ogglist);
    Sys_FreeFileList(wavlist);
}

void Snd_Shutdown(void)
{
    Con_Printf("Snd_Shutdown: deallocating and freeing OpenAL sources and buffers");
    if ((!device || !context) || !snd_cache)
        return;
    
    for (uint32_t i = 0; i < arraylen(sndhash); ++i) {
        if (sndhash[i] && !sndhash[i]->failed) {
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

static int Snd_SoundIsPlaying(nomadsnd_t *snd)
{
    ALint state;

    if (snd->failed)
        return -1;
    
    alGetSourcei(snd->source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING)
        return 1;
    else
        return 0;
}

static qboolean Snd_QueueSfx(nomadsnd_t *snd)
{
    if (Snd_SoundIsPlaying(snd) == 1) {
        Con_Printf(DEV, "Snd_QueueSfx: %s is already playing", snd->name);
        return qfalse;
    }
    if (sfx_queuedCount + 1 >= MAX_SND_QUEUE) {
        Con_Printf("Snd_QueueSfx: overflow");
        return qfalse;
    }

    sfxqueue[sfx_queuedCount++] = snd;
    return qtrue;
}

static void Snd_PlaySound(nomadsnd_t *snd)
{
    int state;

    state = Snd_SoundIsPlaying(snd);
    if (state == -1 || state == 1)
        return;
    else
        alSourcePlay(snd->source);
}

static void Snd_StopSound(nomadsnd_t *snd)
{
    int state;

    state = Snd_SoundIsPlaying(snd);
    if (state == -1 || state == 0)
        return;
    else
        alSourceStop(snd->source);
}

nomadsnd_t* Snd_FetchSnd(const char *name)
{
    return sndhash[Com_GenerateHashValue(name, MAX_SND_HASH)];
}

void Snd_PlayTrack(const char *name)
{
    boost::unique_lock<boost::shared_mutex> lock{sndLock};

    uint64_t hash;
    nomadsnd_t *mus;

    hash = Snd_HashFileName(name);
    mus = sndhash[hash];
    if (!mus) {
        Con_Printf(WARNING, "track '%s' deosn't exist, canceling track", name);
        return;
    }

    if (mus->failed) {
        return;
    }

    if (musicTrack) {
        Con_Printf(DEV, "Snd_PlayTrack: overwriting current music track");
        Snd_StopSound(musicTrack);
        musicTrack = NULL;
    }
    musicTrack = mus;
    Snd_PlaySound(mus);
}

void Snd_PlaySfx(const char *name)
{
    boost::unique_lock<boost::shared_mutex> lock{sndLock};

    uint64_t hash;
    nomadsnd_t *sfx;

    hash = Snd_HashFileName(name);
    sfx = sndhash[hash];
    if (!sfx) {
        Con_Printf(WARNING, "sfx '%s' doesn't exist", name);
        return;
    }

    if (sfx->failed) {
        return;
    }

    if (!Snd_QueueSfx(sfx)) {
        Con_Printf(DEV, "not playing '%s'", name);
    }
}

void Snd_Submit(void)
{
    for (uint32_t i = 0; i < sfx_queuedCount; i++) {
        Snd_PlaySound(sfxqueue[i]);
        sfx_queuedCount--;
        sfxqueue[i] = NULL;
    }
}

void Snd_Init(void)
{
    snd_sfxon = Cvar_Get("snd_sfxon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicon = Cvar_Get("snd_musicon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_sfxvol = Cvar_Get("snd_sfxvol", "1.1f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicvol = Cvar_Get("snd_musicvol", "0.8f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);

    if (!snd_sfxon->i || !snd_musicon->i)
        return;

    Con_Printf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        Cvar_Set("snd_sfxon", "0");
        Cvar_Set("snd_musicon", "0");
        Con_Printf(ERROR, "Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }

    context = alcCreateContext(device, NULL);
    if (!context) {
        Cvar_Set("snd_sfxon", "0");
        Cvar_Set("snd_musicon", "0");
        Con_Printf("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    alcMakeContextCurrent(context);

    memset(sfxqueue, NULL, sizeof(sfxqueue));
    sfx_queuedCount = 0;

    Con_Printf("Snd_Init: successfully initialized sound libraries");
}