#ifndef __SND_LOCAL__
#define __SND_LOCAL__

#pragma once

#include "snd_public.h"
#include <EASTL/vector_map.h>

#define ERRCHECK( call ) { FMOD_RESULT result = call; if ( result != FMOD_OK ) { FMOD_Error( #call, result ); } }
void FMOD_Error( const char *call, FMOD_RESULT result );

#define MAX_SOUND_CHANNELS 1024
#define DISTANCEFACTOR 1.0f
#define MAX_SOUND_SOURCES 1024
#define MAX_MUSIC_QUEUE 12
#define MAX_SOUND_BANKS 528

#define Snd_HashFileName(x) Com_GenerateHashValue((x),MAX_SOUND_SOURCES)

class CSoundSystem;
extern CSoundSystem *sndManager;

class CSoundSource
{
	friend class CSoundSystem;
public:
	CSoundSource( void )
	{ }
	~CSoundSource()
	{ }

	void Release( void );
	bool Load( const char *npath,int64_t nTag );
	
	void Play( bool bLooping = false, uint64_t nTimeOffset = 0, bool bIsWorldSound = false );
	void Stop( void );
	FMOD::Studio::EventInstance *AllocEvent( void );

	inline const char *GetName( void ) const
	{ return m_szName; }
private:
	char m_szName[ MAX_NPATH ];
	CSoundSource *m_pNext;
	int64_t m_nTag;
	qboolean m_bLoaded;
	FMOD::Studio::EventInstance *m_pEmitter;
	FMOD::Studio::EventDescription *m_pData;
};

class CSoundBank
{
public:
	CSoundBank( void )
	{ }
	~CSoundBank()
	{ }

	bool Load( const char *npath );
	void Shutdown( void );

	FMOD::Studio::EventDescription *GetEvent( const char *pName );
private:
	FMOD::Studio::Bank *m_pBank;
	FMOD::Studio::Bank *m_pStrings;

	FMOD::Studio::EventDescription **m_pEventList;
	int m_nEventCount;
};

typedef struct {
	FMOD::Studio::EventInstance *event;
	uint64_t timestamp;
} channel_t;

typedef struct emitter_s {
	linkEntity_t *link;
	channel_t *channel;
	struct emitter_s *next;
	struct emitter_s *prev;
	uint32_t listenerMask;
	float forward;
	float up;
	float velocity;
} emitter_t;

typedef struct {
	linkEntity_t *link;
	uint32_t listenerMask;
	float volume;
} listener_t;

class CSoundWorld
{
public:
	CSoundWorld( void )
	{ }
	~CSoundWorld()
	{ }

	void Init( void );
	void Shutdown( void );
	void Update( void );

	void PlayEmitterSound( nhandle_t hEmitter, float nVolume, uint32_t nListenerMask, sfxHandle_t hSfx );

	void SetListenerVolume( nhandle_t hListener, float nVolume );
	void SetListenerAudioMask( nhandle_t hListener, uint32_t nMask );
	
	void SetEmitterPosition( nhandle_t hEmitter, const vec3_t origin, float forward, float up, float speed );
	void SetEmitterAudioMask( nhandle_t hEmitter, uint32_t nMask );
	void SetEmitterVolume( nhandle_t hEmitter, float nVolume );
	
	nhandle_t PushListener( uint32_t nEntityNumber );
	
	nhandle_t RegisterEmitter( uint32_t nEntityNumber );
	void RemoveEmitter( nhandle_t hEmitter );
private:
	channel_t *AllocateChannel( CSoundSource *pSource );
	void ReleaseChannel( channel_t *pChannel );
	bool LoadOcclusionGeometry( void );

	linkEntity_t *m_pLinks;
	
	emitter_t *m_pszEmitters;
	emitter_t m_EmitterList;
	uint32_t m_nEmitterCount;
	
	// one listener per player
	listener_t m_szListeners[ 4 ];
	uint32_t m_nListenerCount;

	FMOD::Geometry *m_pGeomtryBuffer;

	channel_t *m_pszChannels;
};

typedef struct {
	char szName[ MAX_NPATH ];
	FMOD_GUID guid;
	nhandle_t hBank;
} soundEvent_t;

typedef struct {
	int samplerate;
	int speakerCount;
	FMOD_SPEAKERMODE audioMode;
} soundInfo_t;

class CSoundSystem
{
	friend class CSoundSource;
public:
	CSoundSystem( void )
	{ }
	~CSoundSystem()
	{ }

	void Init( void );
	void Update( void );
	void Shutdown( void );
	void ForceStop( void );

	void SetParameter( const char *pName, float value );

	CSoundSource *LoadSound( const char *npath, int64_t nTag );

	inline CSoundBank **GetBankList( void )
	{ return m_szBanks; }
	inline CSoundSource *GetSound( sfxHandle_t hSfx )
	{ return m_szSources[ hSfx ]; }
	inline uint64_t NumSources( void ) const
	{ return m_nSources; }

	inline static FMOD::Studio::System *GetStudioSystem( void )
	{ return sndManager->m_pStudioSystem; }
	inline static FMOD::System *GetCoreSystem( void )
	{ return sndManager->m_pSystem; }
	inline static FMOD::ChannelGroup *GetSFXGroup( void )
	{ return sndManager->m_pUIGroup; }

	void AddSourceToHash( CSoundSource *pSource );

	eastl::fixed_vector<CSoundSource *, 10> m_szLoopingTracks;

	uint32_t m_nFirstLevelSource;
	uint32_t m_nLevelSources;
private:
	friend void *Sound_Thread( void *arg );

	bool LoadBank( const char *pName );

	FMOD::Studio::System *m_pStudioSystem;
	FMOD::System *m_pSystem;

	CSoundSource *m_szSources[ MAX_SOUND_SOURCES ];
	CSoundBank *m_szBanks[ MAX_SOUND_BANKS ];

	uint64_t m_nSources;

	FMOD::ChannelGroup *m_pUIGroup;
	FMOD::ChannelGroup *m_pSFXGroup;

	soundInfo_t m_AudioInfo;
};

extern cvar_t *snd_musicOn;
extern cvar_t *snd_effectsOn;
extern cvar_t *snd_musicVolume;
extern cvar_t *snd_effectsVolume;
extern cvar_t *snd_masterVolume;
extern cvar_t *snd_debugPrint;
extern cvar_t *snd_noSound;
extern cvar_t *snd_muteUnfocused;
extern cvar_t *snd_maxChannels;
extern CSoundWorld s_SoundWorld;

#endif
