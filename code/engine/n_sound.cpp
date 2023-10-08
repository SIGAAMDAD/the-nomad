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
static qboolean clearedQueue;

cvar_t *snd_musicvol;
cvar_t *snd_sfxvol;
cvar_t *snd_musicon;
cvar_t *snd_sfxon;

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

#define SND_SFX 0
#define SND_MUSIC 1

static sf_count_t SndFile_Read(void *data, sf_count_t size, void *file) {
    return FS_Read(data, size, *(file_t *)f);
}

static sf_count_t SndFile_Tell(void *file) {
    return FS_FileTell(*(file_t *)file);
}

static sf_count_t SndFile_GetFileLen(void *file) {
    return FS_FileLength(*(file_t *)file);
}

static sf_count_t SndFile_Seek(sf_count_t offset, int whence, void *file) {
    file_t f = *(file_t *)file;
    switch (whence) {
    case SEEK_SET:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_SET);
    case SEEK_CUR:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_CUR);
    case SEEK_END:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_END);
    };
}

static nomadsnd_t *Snd_LoadFile(const char *path, sfxHandle_t *sfx, int type)
{
    file_t f;
    SF_INFO fdata;
    SF_VIRTUAL_IO vio;
    SNDFILE *sf;
    nomadsnd_t *snd;
    sf_count_t readCount;
    uint64_t hash;

    // check if the sound already exists
    hash = Snd_HashFileName(path);
    if (sndhash[hash]) {
        return sndhash[hash];
    }
    *sfx = (sfxHandle_t)hash;

    if (type == SND_SFX) {
        snd = (nomadsnd_t *)Z_Malloc(sizeof(*snd), TAG_SFX);
    }
    else if (type == SND_MUSIC) {
        snd = (nomadsnd_t *)Z_Malloc(sizeof(*snd), TAG_MUSIC);
    }
    memset(snd, 0, sizeof(*snd));

    sndhash[hash] = snd;
    snd->failed = qtrue;
    N_strncpyz(snd->name, path, sizeof(snd->name));

    f = FS_FOpenRead(path);
    if (f == FS_INVALID_HANDLE) {
        Con_Printf(COLOR_YELLOW "WARNING: Failed to load sound file %s", path);
        return NULL;
    }

    vio.read = SndFile_Read;
    vio.write = NULL; // not needed in this case
    vio.get_filelen = SndFile_GetFileLen;
    vio.seek = SndFile_Seek;
    vio.tell = SndFile_Tell;

    sf = sf_open_virtual(&vio, SFM_READ, &fdata, &f);
    if (!sf) {
        N_Error(ERR_FATAL, "Snd_LoadFile: sf_open_virtual failed, error: %s", sf_strerror(sf));
    }

    snd->sndbuf = (short *)Hunk_Alloc(sizeof(*snd->sndbuf) * snd->length, h_low);
    readCount = sf_read_short(sf, snd->sndbuf, sizeof(*snd->sndbuf) * fdata.frames * fdata.channels);
    if (readCount != fileLen) {
        N_Error(ERR_FATAL, "Snd_LoadFile: sf_read_short failed, error: %s", sf_strerror(sf));
    }
    sf_close(sf);

    alGenSources(1, (ALuint *)&snd->source);
    alGenBuffers(1, (ALuint *)&snd->buffer);
    alBufferData(snd->buffer, Snd_Format(snd), snd->sndbuf, sizeof(*snd->sndbuf) * snd->length, snd->samplerate);
    alSourcei(snd->source, AL_BUFFER, snd->buffer);

    FS_FClose(f);

    snd->failed = qfalse;
    
    return snd;
}

sfxHandle_t Snd_RegisterSfx(const char *npath)
{
    nomadsnd_t *snd;
    sfxHandle_t sfx;

    if (strlen(npath) >= MAX_GDR_PATH) {
        Con_Printf(COLOR_YELLOW "Snd_RegisterSfx: name '%s' too long", npath);
        return -1;
    }

    snd = Snd_LoadFile(npath, &sfx, SND_SFX);
    if (!snd) {
        return -1;
    }
    return sfx;
}

sfxHandle_t Snd_RegisterTrack(const char *npath)
{
    nomadsnd_t *track;
    sfxHandle_t id;

    if (strlen(npath) >= MAX_GDR_PATH) {
        Con_Printf(COLOR_YELLOW "Snd_RegisterTrack: name '%s' too long\n", npath);
        return -1;
    }

    track = Snd_LoadFile(npath, &id, SND_MUSIC);
    if (!track) {
        return -1;
    }
    return id;
}

void Snd_Restart(void)
{
    Snd_Shutdown(qfalse);

    // reinitialize
    for (uint32_t i = 0; i < arraylen(sndhash); ++i) {
        if (sndhash[i] && !sndhash[i]->failed) {
            alGenSources(1, (ALuint *)&sndhash[i]->source);
            alGenBuffers(1, (ALuint *)&sndhash[i]->buffer);
            alBufferData(sndhash[i]->buffer, Snd_Format(sndhash[i]), sndhash[i]->sndbuf,
                sizeof(*sndhash[i]->sndbuf) * sndhash[i]->length, sndhash[i]->samplerate);
            alSourcei(sndhash[i]->source, AL_BUFFER, sndhash[i]->buffer);
        }
    }
}

//
// Snd_ClearMem: clears all memory allocated to sound stuff
//
void Snd_ClearMem(void)
{
    Snd_Shutdown(qfalse);
    Z_FreeTags(TAG_SFX, TAG_MUSIC);
}

void Snd_Shutdown(qboolean destroyContext)
{
    Con_Printf("Snd_Shutdown: deallocating and freeing OpenAL sources and buffers\n");
    if ((!device || !context) || !snd_cache)
        return;
    
    Snd_StopAll();
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
    if (destroyContext) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
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

static qboolean Snd_PlaySound(nomadsnd_t *snd)
{
    int state;

    state = Snd_SoundIsPlaying(snd);
    if (state == -1 || state == 1)
        return qfalse;
    else
        alSourcePlay(snd->source);
    
    return qtrue;
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

    if (!snd_musicon->i) {
        Con_DPrintf("snd_musicon->i != 1\n");
        return;
    }

    hash = Snd_HashFileName(name);
    mus = sndhash[hash];
    if (!mus) {
        N_Error(ERR_DROP, "track '%s' deosn't exist, canceling track\n", name);
    }

    if (mus->failed) {
        return;
    }

    if (musicTrack) {
        Con_DPrintf("Snd_PlayTrack: overwriting current music track\n");
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

    if (!snd_sfxon->i) {
        Con_DPrintf("snd_sfxon->i != 1\n");
        return;
    }

    hash = Snd_HashFileName(name);
    sfx = sndhash[hash];
    if (!sfx) {
        N_Error(ERR_DROP, "sfx '%s' doesn't exist", name);
    }

    if (sfx->failed) {
        return;
    }

    if (!Snd_QueueSfx(sfx)) {
        Con_DPrintf("not playing '%s'\n", name);
    }
}

void Snd_PlayTrack(sfxHandle_t sfx)
{
    boost::unique_lock<boost::shared_mutex> lock{sndLock};

    uint64_t hash;
    nomadsnd_t *mus;

    if (!snd_musicon->i) {
        Con_DPrintf("snd_musicon->i != 1\n");
        return;
    }

    mus = sndhash[sfx];
    if (!mus) {
        N_Error(ERR_DROP, "Snd_PlayTrack: invalid handle, canceling track\n");
    }

    if (mus->failed) {
        return;
    }

    if (musicTrack) {
        Con_DPrintf("Snd_PlayTrack: overwriting current music track\n");
        Snd_StopSound(musicTrack);
        musicTrack = NULL;
    }
    musicTrack = mus;
    Snd_PlaySound(mus);
}

void Snd_PlaySfx(sfxHandle_t sfx)
{
    boost::unique_lock<boost::shared_mutex> lock{sndLock};
    nomadsnd_t *snd;

    if (!snd_sfxon->i) {
        Con_DPrintf("snd_sfxon->i != 1\n");
        return;
    }

    snd = sndhash[sfx];
    if (!snd) {
        N_Error(ERR_DROP, "Snd_PlaySfx: bad handle");
    }

    if (snd->failed) {
        return;
    }

    if (!Snd_QueueSfx(snd)) {
        Con_DPrintf("Snd_PlaySfx: playing '%s'\n", snd->name);
    }
}


void Snd_Submit(void)
{
    // update any parameters if the cvars have been changed
    if (Cvar_CheckGroup(CVG_SOUND)) {
    }

    if (!snd_musicon->i && !snd_sfxon->i) {
        if (!clearedQueue) {
            memset(sfxqueue, NULL, sizeof(sfxqueue));
            clearedQueue = qtrue;
        }
        Snd_StopAll();
    }

    for (uint32_t i = 0; i < sfx_queuedCount; i++) {
        if (Snd_PlaySound(sfxqueue[i])) {
            sfx_queuedCount--;
            sfxqueue[i] = NULL;
        }
    }
}

void Snd_StopAll(void)
{
    static qboolean stoppedAll = qfalse;
    if (stoppedAll) {
        return; // already called
    }
    for (uint32_t i = 0; i < arraylen(sndhash); ++i) {
        if (sndhash[i] && Snd_SoundIsPlaying(sndhash[i])) {
            Snd_StopSound(sndhash[i]);
        }
    }
    stoppedAll = qtrue;
}

void Snd_DisableSounds(void)
{
    Snd_StopAll();
    Snd_ClearMem();
}

void Snd_Init(void)
{
    snd_sfxon = Cvar_Get("snd_sfxon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicon = Cvar_Get("snd_musicon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_sfxvol = Cvar_Get("snd_sfxvol", "1.1f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicvol = Cvar_Get("snd_musicvol", "0.8f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);

    if (!snd_sfxon->i || !snd_musicon->i)
        return;

    Con_Printf("Snd_Init: initializing OpenAL and libsndfile\n");
    device = alcOpenDevice(NULL);
    if (!device) {
        Cvar_Set("snd_sfxon", "0");
        Cvar_Set("snd_musicon", "0");
        Con_Printf(COLOR_RED "Snd_Init: alcOpenDevice returned NULL, turning off sound\n");
        return;
    }

    context = alcCreateContext(device, NULL);
    if (!context) {
        Cvar_Set("snd_sfxon", "0");
        Cvar_Set("snd_musicon", "0");
        Con_Printf(COLOR_RED "Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s\n",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    alcMakeContextCurrent(context);

    memset(sfxqueue, NULL, sizeof(sfxqueue));
    clearedQueue = qfalse;
    sfx_queuedCount = 0;
}