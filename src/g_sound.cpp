#include "n_shared.h"
#include "g_game.h"

#define alCall(x) x; if (alGetError() != AL_NO_ERROR) N_Error("%s: OpenAL error occurred, error message: %s",FUNC_SIG,alGetString(alGetError()))

static ALCdevice* device;
static ALCcontext* context;

nomadsnd_t* mus_cache;
nomadsnd_t* sfx_cache;

void Snd_Kill()
{
    con.ConPrintf("Snd_Kill: deallocating and freeing OpenAL sources and buffers");
    if ((!scf::audio::music_on && !scf::audio::sfx_on) || (!device || !context) || !sfx_cache)
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
        con.ConError("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: {}",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    assert(context);
    alcMakeContextCurrent(context);

    con.ConPrintf("Snd_Init: successfully initialized sound libraries");

    scf::audio::sfx_on = true;
    scf::audio::music_on = true;
}