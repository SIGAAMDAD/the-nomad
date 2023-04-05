#include "n_shared.h"
#include "g_game.h"

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <sndfile.hh>

enum : uint32_t
{
    sfx_shotty_dryfire,
    sfx_rifle_dryfire,
    sfx_pistol_dryfire,
    sfx_adb_shot,
    sfx_fab_shot,
    sfx_qs_shot,
    sfx_rifle_shot,
    sfx_plasma_shot,
    sfx_playr_hurt0,

    NUMSFX
};

constexpr uint32_t numsfx = NUMSFX;

#define alCall(x) x; if (alGetError() != AL_NO_ERROR) N_Error("%s: OpenAL error occurred, error message: %s",__func__,alGetString(alGetError()))

static ALCdevice* device;
static ALCcontext* context;

typedef struct nomadsnd_s
{
    ALuint source;
    ALuint buffer;
    char name[180];

    bool queued = false;
    bool failed = false; // if the pre-caching effort failed for this specific sound
} nomadsnd_t;

constexpr const char sfxinfo[numsfx][180] = {
    "PLDRYFR0.ogg",
    "PLDRYFR1.ogg",
    "PLDRYFR2.ogg",
    "ADBSHOT.wav",
    "FABSHOT.wav",
};

static nomadsnd_t* sfx_cache;
static nomadsnd_t* music_cache;

static void I_CacheSFX()
{
    if (!scf::audio::sfx_on)
        return;

    LOG_INFO("running I_CacheSFX for sfx pre-allocation");
    sfx_cache = (nomadsnd_t *)Z_Malloc(sizeof(nomadsnd_t) * numsfx, TAG_CACHE, &sfx_cache);
    assert(sfx_cache);
    LOG_INFO("successfully allocated sfx_cache pointer via the heap");

    int16_t buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    for (uint32_t i = 0; i < numsfx; ++i) {
        nomadsnd_t* sfx = &sfx_cache[i];
        {
            SF_INFO fdata;
            std::vector<int16_t> rdbuf;
            SNDFILE* sf = sf_open(sfxinfo[i], SFM_READ, &fdata);
            if (!sf) {
                LOG_WARN("I_CacheSFX: failed to open sfx file {}, canceling caching of this sfx", sfxinfo[i]);
                goto skip;
            }
            assert(sf);
            size_t read;
            while ((read = sf_read_short(sf, buffer, sizeof(buffer))) != 0) {
                rdbuf.insert(rdbuf.end(), buffer, buffer + read);
            }
            sf_close(sf);
            alCall(alGenSources(1, &sfx->source));
            alCall(alGenBuffers(1, &sfx->buffer));
            alCall(alBufferData(sfx->buffer, fdata.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                &rdbuf.front(), rdbuf.size() * sizeof(int16_t), fdata.samplerate));
            alCall(alSourcei(sfx->source, AL_BUFFER, sfx->buffer));
            alCall(alSourcef(sfx->source, AL_GAIN, scf::audio::sfx_vol));
        }
        sfx->failed = false;
        sfx->queued = false;
        N_strncpy(sfx->name, sfxinfo[i], 180);
skip:
        sfx->failed = true;
    }
    LOG_INFO("I_CacheSFX succeeded");
}

static void I_CacheMusic()
{
}

void Snd_Kill()
{
    con.ConPrintf("Snd_Kill: deallocating and freeing OpenAL sources and buffers");
    if (!scf::audio::music_on && !scf::audio::sfx_on)
        return;

    if (scf::audio::sfx_on) {
        for (uint32_t i = 0; i < numsfx; ++i) {
            if (!sfx_cache[i].failed) {
                alSourcei(sfx_cache[i].source, AL_BUFFER, 0);
                alDeleteBuffers(1, &sfx_cache[i].buffer);
                alDeleteSources(1, &sfx_cache[i].source);
            }
        }
    }
    if (scf::audio::music_on) {
    }
    if (scf::audio::sfx_on || scf::audio::music_on) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }
}

void P_PlaySFX(uint32_t sfx)
{
    assert(sfx < numsfx);
    sfx_cache[sfx].queued = true;
}

static std::mutex snd_mutex;

void G_RunSound()
{
    std::lock_guard lock(snd_mutex);
    if (!scf::audio::music_on)
        return;
    
    if (!scf::audio::sfx_on)
        return;

    for (uint32_t i = 0; i < numsfx; ++i) {
        if (sfx_cache[i].queued) {
            alSourcePlay(sfx_cache[i].source);
            sfx_cache[i].queued = false;
        }
    }
}

void Snd_Init()
{
    con.ConPrintf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        scf::audio::sfx_on = false;
        scf::audio::music_on = false;
        con.ConError("Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }
    assert(device);

    context = alcCreateContext(device, NULL);
    if (!context) {
        scf::audio::sfx_on = false;
        scf::audio::music_on = false;
        con.ConError("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    assert(context);
    alcMakeContextCurrent(context);

    con.ConPrintf("Snd_Init: successfully initialized sound libraries");

    scf::audio::sfx_on = true;
    scf::audio::music_on = true;

    I_CacheSFX();
    I_CacheMusic();
}