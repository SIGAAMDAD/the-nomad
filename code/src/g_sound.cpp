#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include "n_shared.h"
#include "g_game.h"
#include "g_sound.h"

static ALCdevice* device;
static ALCcontext* context;

void Snd_Kill()
{
    Con_Printf("Snd_Kill: deallocating and freeing OpenAL sources and buffers");
    if ((!snd_musicon && !snd_sfxon) || (!device || !context) || !snd_cache)
        return;

    if (snd_sfxon || snd_musicon) {
        for (uint32_t i = 0; i < numsfx; ++i) {
            if (!snd_cache[i].failed) {
                alSourcei(snd_cache[i].source, AL_BUFFER, 0);
                alDeleteBuffers(1, &snd_cache[i].buffer);
                alDeleteSources(1, &snd_cache[i].source);
            }
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

static std::mutex snd_mutex;

void G_RunSound()
{
    std::lock_guard lock(snd_mutex);
    for (uint32_t i = 0; i < sndcache_size; i++) {
        if (snd_cache[i].queued && !snd_cache[i].failed) {
            alSourcePlay(snd_cache[i].source);
        }
    }
}

void Snd_Init()
{
    Con_Printf("Snd_Init: initializing OpenAL and libsndfile");
    device = alcOpenDevice(NULL);
    if (!device) {
        snd_sfxon = qfalse;
        snd_musicon = qfalse;
        Con_Printf("Snd_Init: alcOpenDevice returned NULL, turning off sound");
        return;
    }
    assert(device);

    context = alcCreateContext(device, NULL);
    if (!context) {
        snd_sfxon = qfalse;
        snd_musicon = qfalse;
        Con_Printf("Snd_Init: alcCreateContext returned NULL, turning off sound, error message: %s",
            alcGetString(device, alcGetError(device)));
        alcCloseDevice(device);
        return;
    }
    assert(context);
    alcMakeContextCurrent(context);

    Con_Printf("Snd_Init: successfully initialized sound libraries");

    snd_sfxon = qtrue;
    snd_musicon = qtrue;
}