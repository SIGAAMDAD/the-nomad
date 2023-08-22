#ifndef _N_SOUND_
#define _N_SOUND_

#pragma once

void I_CacheAudio(void);
#ifdef __cplusplus
void Snd_PlayTrack(const char *name);
void Snd_PlaySfx(const char *name);
#endif
void Snd_PlayTrack(sfxHandle_t sfx);
void Snd_PlaySfx(sfxHandle_t sfx);
void Snd_Init(void);
void Snd_Submit(void);
void Snd_Restart(void);
void Snd_Shutdown(qboolean destroyContext);
sfxHandle_t Snd_RegisterSfx(const char *npath);

#endif