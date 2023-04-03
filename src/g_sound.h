#ifndef _G_SOUND_
#define _G_SOUND_

#pragma once

void P_PlaySFX(uint32_t sfx);
void S_PlayMusic(uint32_t music);
void Snd_Init();
void Snd_Kill();
void G_RunSound();

#endif