#ifndef __SND_LOCAL__
#define __SND_LOCAL__

#pragma once

#include "snd_public.h"

#define ERRCHECK( call ) { FMOD_RESULT result = call; if ( result != FMOD_OK ) { FMOD_Error( #call, result ); } }
void FMOD_Error( const char *call, FMOD_RESULT result );

#define MAX_SOUND_CHANNELS 1024
#define DISTANCEFACTOR 1.0f
#define MAX_SOUND_SOURCES 2048
#define MAX_MUSIC_QUEUE 12

#define Snd_HashFileName(x) Com_GenerateHashValue((x),MAX_SOUND_SOURCES)

class CSoundSystem
{
public:
	CSoundSystem( void );
	~CSoundSystem();

	void Init( void );
	void Update( void );
	void Shutdown( void );

	FMOD::Sound *LoadSound( const char *npath );

	inline FMOD::Sound *GetSound( sfxHandle_t hSfx )
	{ return m_szSources[ hSfx ]; }
private:
	FMOD::Studio::System *m_pStudioSystem;
	FMOD::System *m_pSystem;

	int m_nNextChannelId;

	FMOD::Sound *m_szSources[ MAX_SOUND_SOURCES ];

	eastl::map<string_t, FMOD::Sound *> SoundMap;
	eastl::map<int, FMOD_CHANNEL *> ChannelMap;
	eastl::map<string_t, FMOD::Studio::EventInstance *> EventMap;
	eastl::map<string_t, FMOD::Studio::Bank *> BankMap;

	FMOD::ChannelGroup *m_pMasterGroup;
};

#endif