#include "snd_local.h"

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
	int ErrorCheck( FMOD_RESULT result );

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

CSoundSystem::CSoundSystem( void )
{
	m_pStudioSystem = NULL;
	m_pSystem = NULL;

	ErrorCheck( FMOD::Studio::System::create( &m_pStudioSystem ) );
	ErrorCheck( m_pStudioSystem->getCoreSystem( &m_pSystem ) );
	ErrorCheck( m_pSystem->setSoftwareFormat( 48000, FMOD_SPEAKERMODE_STEREO, 0 ) );
	ErrorCheck( m_pSystem->set3DSettings( 1.0f, DISTANCEFACTOR, 0.5f ) );
	ErrorCheck( m_pStudioSystem->initialize( 32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL ) );
	ErrorCheck( m_pSystem->getMasterChannelGroup( &m_pMasterGroup ) );
}

CSoundSystem::~CSoundSystem()
{
	ErrorCheck( m_pStudioSystem->unloadAll() );
	ErrorCheck( m_pStudioSystem->release() );
}

void CSoundSystem::Update( void )
{
	ErrorCheck( m_pStudioSystem->update() );
}

FMOD::Sound *CSoundSystem::LoadSound( const char *npath )
{
	FMOD::Sound *pSound;

	m_pSystem->createSound( npath, FMOD_2D, NULL, &pSound );

	return pSound;
}

static CSoundSystem *g_pSoundSystem;

void Snd_DisableSounds( void );
void Snd_StopAll( void );
void Snd_PlaySfx( sfxHandle_t sfx );
void Snd_StopSfx( sfxHandle_t sfx );
void Snd_Init( void );
void Snd_Restart( void );
void Snd_Shutdown( void );
void Snd_Update( int msec );

sfxHandle_t Snd_RegisterTrack( const char *npath )
{
}

sfxHandle_t Snd_RegisterSfx( const char *npath )
{
	sfxHandle_t hSfx;

	hSfx = Snd_HashFileName( npath );

	return hSfx;
}

void Snd_PlayWorldSfx( const vec3_t origin, sfxHandle_t hSfx )
{
	FMOD::Sound *pSound;

	pSound = g_pSoundSystem->GetSound( hSfx );

	pSound->setMode( FMOD_LOOP_OFF );
	pSound->set3DMinMaxDistance( 0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR );
}

void Snd_SetWorldListener( const vec3_t origin )
{
}

void Snd_ClearLoopingTracks( void );
void Snd_AddLoopingTrack( sfxHandle_t handle, uint64_t timeOffset = 0 );