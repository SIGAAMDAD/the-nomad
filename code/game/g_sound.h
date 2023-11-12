#ifndef _N_SOUND_
#define _N_SOUND_

#pragma once

void Snd_DisableSounds( void );
void Snd_StopAll( void );
void Snd_PlayTrack( sfxHandle_t sfx );
void Snd_PlaySfx( sfxHandle_t sfx );
void Snd_StopSfx( sfxHandle_t sfx );
void Snd_Init( void );
void Snd_Restart( void );
void Snd_Shutdown( void );
void Snd_Update( int msec );
sfxHandle_t Snd_RegisterTrack( const char *npath );
sfxHandle_t Snd_RegisterSfx( const char *npath );
void Snd_SetLoopingTrack( sfxHandle_t handle );
void Snd_ClearLoopingTrack( void );

#endif