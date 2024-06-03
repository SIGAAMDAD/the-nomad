#ifndef __N_SOUND__
#define __N_SOUND__

#pragma once

//#define USE_QUAKE3_SOUND
#ifndef USE_QUAKE3_SOUND

void Snd_DisableSounds( void );
void Snd_StopAll( void );
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
void Snd_PlayWorldSfx( const vec3_t origin, sfxHandle_t hSfx );
void Snd_SetWorldListener( const vec3_t origin );

void Snd_StartupThread( int msec );
void Snd_JoinThread( void );

#endif

#endif