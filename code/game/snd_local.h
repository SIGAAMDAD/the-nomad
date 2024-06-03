#ifndef __SND_LOCAL__
#define __SND_LOCAL__

#pragma once

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <ALsoft/alext.h>
#include <sndfile.h>

#define MAX_SOUNDS 4096

typedef struct {
    int format;
    int channels;
    int samplerate;
    int avgBytesPerSec;
    int blockAlign;
    int bitsPerSample;
} sfxInfo_t;

typedef struct sfx_s {
    char name[MAX_NPATH];

    byte *nonCacheData;

    sfxInfo_t objectInfo;
    uint32_t objectMemSize;
    uint32_t objectSize;
    ALuint openalBuffer;

    qboolean hardwareBuffer;
    qboolean purged;

    struct sfx_s *next;
} sfx_t;

typedef struct {
	ALuint			handle;
	int				startTime;
	sfx_t	        *chan;
	bool			inUse;
	bool			looping;
	bool			stereo;
} openalSource_t;

typedef struct {
    sfx_t *soundList[MAX_SOUNDS];
    uint64_t numSounds;

    ALCdevice *openalDevice;
    ALCcontext *openalContext;
    ALsizei openalSourceCount;

    qboolean muted;

	int resetRetryCount;
	uint32_t lastCheckTime;
} soundSystem_t;

extern idDynamicBlockAlloc<byte, 1<<20, 1<<10> soundCacheAllocator;
extern soundSystem_t *soundSystem;

extern cvar_t *snd_force22kHz;

void SndCache_ReloadSounds( bool force );

extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERF alFilterf;
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALCRESETDEVICESOFT alcResetDeviceSOFT; // needs ALC_SOFT_HRTF extension

#endif