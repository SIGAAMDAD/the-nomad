#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include "n_shared.h"
#include "g_game.h"
#include "n_scf.h"
#include "g_sound.h"

static ALCdevice* device;
static ALCcontext* context;

void Snd_Kill()
{
    Con_Printf("Snd_Kill: deallocating and freeing OpenAL sources and buffers");
    if ((!N_strtobool(snd_musicon.value) && !N_strtobool(snd_sfxon.value)) || (!device || !context) || !snd_cache)
        return;

    if (N_strtobool(snd_sfxon.value) || N_strtobool(snd_musicon.value)) {
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