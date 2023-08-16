#ifndef _N_SOUND_
#define _N_SOUND_

#pragma once

void I_CacheAudio(void);
void Snd_PlayTrack(const char *name);
void Snd_PlaySfx(const char *name);
void Snd_Init(void);
void Snd_Submit(void);
void Snd_Shutdown(void);

#endif