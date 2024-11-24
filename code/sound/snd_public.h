#ifndef __SND_PUBLIC__
#define __SND_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <fmod/fmod.hpp>
#include <fmod/fmod_studio.hpp>
#include <EASTL/map.h>
#include "../module_lib/module_public.h"

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
void Snd_PlayWorldSfx( const vec3_t origin, sfxHandle_t hSfx );
void Snd_SetWorldListener( const vec3_t origin );

void Snd_ClearLoopingTracks( void );
void Snd_AddLoopingTrack( sfxHandle_t handle, uint64_t timeOffset = 0 );

void Snd_StartupThread( int msec );
void Snd_JoinThread( void );

#endif