#ifndef _G_SOUND_
#define _G_SOUND_

#pragma once

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

constexpr const char sfxinfo[numsfx][180] = {
    "PLDRYFR0.ogg",
    "PLDRYFR1.ogg",
    "PLDRYFR2.ogg",
    "ADBSHOT.wav",
    "FABSHOT.wav",
};

typedef struct nomadsnd_s
{
    ALuint source;
    ALuint buffer;
    char name[80];

    int samplerate;
    int channels;
    int length;
    short* sndbuf;

    bool queued = false;
    bool failed = false; // if the pre-caching effort failed for this specific sound
} nomadsnd_t;

extern uint32_t sndcache_size;
extern nomadsnd_t* snd_cache;

void I_CacheAudio(void *bffinfo);
void P_PlaySFX(uint32_t sfx);
void S_PlayMusic(uint32_t music);
void Snd_Init();
void Snd_PlayTrack(const char *name);
void Snd_Kill();
void G_RunSound();

#endif