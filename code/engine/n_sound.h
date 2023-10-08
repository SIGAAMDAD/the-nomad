#ifndef _N_SOUND_
#define _N_SOUND_

#pragma once

void Snd_DisableSounds(void);
void Snd_StopAll(void);
void Snd_ClearMem(void);
void Snd_PlayTrack(sfxHandle_t sfx);
void Snd_PlaySfx(sfxHandle_t sfx);
void Snd_Init(void);
void Snd_Submit(void);
void Snd_Restart(void);
void Snd_Shutdown(qboolean destroyContext);
sfxHandle_t Snd_RegisterSfx(const char *npath);

#endif