#include "snd_local.h"

void FMOD_Error( const char *call, FMOD_RESULT result )
{
	const char *err;
	int isError;

	isError = 0;

	switch ( result ) {
	case FMOD_ERR_CHANNEL_ALLOC:
		err = "Error allocating channel";
		isError = 1;
		break;
	case FMOD_ERR_CHANNEL_STOLEN:
		isError = 1;
		break;
	case FMOD_ERR_DMA:
		break;
	
	};
	
	if ( isError == 0 ) {
		N_Error( ERR_DROP, "FMOD API Error: %s", err );
	} else if ( isError == 1 ) {
		N_Error( ERR_FATAL, "FMOD API Error: %s", err );
	} else {
		Con_Printf( COLOR_RED "FMOD API Error: %s\n", err );
	}
}

CSoundSystem::CSoundSystem( void )
{
	m_pStudioSystem = NULL;
	m_pSystem = NULL;

	ERRCHECK( FMOD::Studio::System::create( &m_pStudioSystem ) );
	ERRCHECK( m_pStudioSystem->getCoreSystem( &m_pSystem ) );
	ERRCHECK( m_pSystem->setSoftwareFormat( 48000, FMOD_SPEAKERMODE_5POINT1, 0 ) );
	ERRCHECK( m_pSystem->set3DSettings( 1.0f, DISTANCEFACTOR, 0.5f ) );
	ERRCHECK( m_pStudioSystem->initialize( 32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL ) );
	ERRCHECK( m_pSystem->getMasterChannelGroup( &m_pMasterGroup ) );
}

CSoundSystem::~CSoundSystem()
{
	ERRCHECK( m_pStudioSystem->unloadAll() );
	ERRCHECK( m_pStudioSystem->release() );
}

void CSoundSystem::Update( void )
{
	ERRCHECK( m_pStudioSystem->update() );
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