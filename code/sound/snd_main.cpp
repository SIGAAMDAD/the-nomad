#include "snd_local.h"
#include <EASTL/atomic.h>

CSoundSystem *sndManager = NULL;

cvar_t *snd_musicOn;
cvar_t *snd_effectsOn;
cvar_t *snd_musicVolume;
cvar_t *snd_effectsVolume;
cvar_t *snd_masterVolume;
cvar_t *snd_debugPrint;
cvar_t *snd_noSound;
cvar_t *snd_muteUnfocused;
cvar_t *snd_maxChannels;
cvar_t *snd_speakerMode;
cvar_t *snd_debug;

CSoundWorld s_SoundWorld;

static FMOD::Studio::System *s_pStudioSystem;
static FMOD::System *s_pCoreSystem;

static fileHandle_t fmodLogFile;

void FMOD_Error( const char *call, FMOD_RESULT result )
{
	int i;

	static const struct {
		int num;
		int severity;
		const char *str;
	} fmod_errors[] = {
		{ FMOD_ERR_BADCOMMAND, 2, "Tried to call a function on a data type that does not allow this type of functionality." },
		{ FMOD_ERR_CHANNEL_ALLOC, 1, "Error trying to allocate a Channel." },
		{ FMOD_ERR_CHANNEL_STOLEN, 0, "The specified Channel has been reused to play another Sound." },
		{ FMOD_ERR_DMA, 2, "DMA Failure. See debug output for more information." },
		{ FMOD_ERR_DSP_CONNECTION, 2, "DSP connection error. Connection possibly caused a cyclic dependency or connected dsps with incompatible buffer counts." },
		{ FMOD_ERR_DSP_DONTPROCESS, 2, "DSP return code from a DSP process query callback. Tells mixer not to call the process callback and therefore not consume CPU. Use this to optimize the DSP graph." },
		{ FMOD_ERR_DSP_FORMAT, 2, "DSP Format error. A DSP unit may have attempted to connect to this network with the wrong format, or a matrix may have been set with the wrong size if the target unit has a specified channel map." },
		{ FMOD_ERR_DSP_INUSE, 2, "DSP is already in the mixer's DSP network. It must be removed before being reinserted or released." },
		{ FMOD_ERR_DSP_NOTFOUND, 2, "DSP connection error. Couldn't find the DSP unit specified." },
		{ FMOD_ERR_DSP_RESERVED, 2, "DSP operation error. Cannot perform operation on this DSP as it is reserved by the system.s" },
		{ FMOD_ERR_DSP_SILENCE, 2, "DSP return code from a DSP process query callback. Tells mixer silence would be produced from read, so go idle and not consume CPU. Use this to optimize the DSP graph." },
		{ FMOD_ERR_DSP_TYPE, 2, "DSP operation cannot be performed on a DSP of this type." },
		{ FMOD_ERR_FILE_BAD, 2, "Error loading file." },
		{ FMOD_ERR_FILE_COULDNOTSEEK, 2, "Couldn't perform seek operation. This is a limitation of the medium (ie netstreams) or the file format." },
		{ FMOD_ERR_FILE_DISKEJECTED, 2, "Media was ejected while reading." },
		{ FMOD_ERR_FILE_EOF, 2, "End of file unexpectedly reached while trying to read essential data (truncated?)." },
		{ FMOD_ERR_FILE_ENDOFDATA, 2, "End of current chunk reached while trying to read data." },
		{ FMOD_ERR_FILE_NOTFOUND, 2, "File not found." },
		{ FMOD_ERR_FORMAT, 2, "Unsupported file or audio format." },
		{ FMOD_ERR_HEADER_MISMATCH, 2, "There is a version mismatch between the FMOD header and either the FMOD Studio library or the FMOD Core library." },
		{ FMOD_ERR_HTTP, 2, "A HTTP error occurred. This is a catch-all for HTTP errors not listed elsewhere." },
		{ FMOD_ERR_HTTP_ACCESS, 2, "The specified resource requires authentication or is forbidden." },
		{ FMOD_ERR_HTTP_PROXY_AUTH, 2, "Proxy authentication is required to access the specified resource." },
		{ FMOD_ERR_HTTP_SERVER_ERROR, 2, "A HTTP server error occurred." },
		{ FMOD_ERR_HTTP_TIMEOUT, 2, "The HTTP request timed out." },
		{ FMOD_ERR_INITIALIZATION, 2, "FMOD was not initialized correctly to support this function." },
		{ FMOD_ERR_INITIALIZED, 2, "Cannot call this command after System::init." },
		{ FMOD_ERR_INTERNAL, 2, "An error occured in the FMOD system. Use the logging version of FMOD for more information." },
		{ FMOD_ERR_INVALID_FLOAT, 2, "Value passed in was a NaN, Inf or denormalized float." },
		{ FMOD_ERR_INVALID_HANDLE, 2, "An invalid object handle was used." },
		{ FMOD_ERR_INVALID_PARAM, 2, "An invalid parameter was passed to this function." },
		{ FMOD_ERR_INVALID_POSITION, 2, "An invalid seek position was passed to this function." },
		{ FMOD_ERR_INVALID_SPEAKER, 2, "An invalid speaker was passed to this function based on the current speaker mode." },
		{ FMOD_ERR_INVALID_SYNCPOINT, 2, "The syncpoint did not come from this Sound handle." },
		{ FMOD_ERR_INVALID_THREAD, 2, "Tried to call a function on a thread that is not supported." },
		{ FMOD_ERR_INVALID_VECTOR, 2, "The vectors passed in are not unit length, or perpendicular." },
		{ FMOD_ERR_MAXAUDIBLE, 2, "Reached maximum audible playback count for this Sound's SoundGroup." },
		{ FMOD_ERR_MEMORY, 2, "Not enough memory or resources." },
		{ FMOD_ERR_MEMORY_CANTPOINT, 2, "Can't use FMOD_OPENMEMORY_POINT on non PCM source data, or non mp3/xma/adpcm data if FMOD_CREATECOMPRESSEDSAMPLE was used." },
		{ FMOD_ERR_NEEDS3D, 2, "Tried to call a command on a 2D Sound when the command was meant for 3D Sound." },
		{ FMOD_ERR_NEEDSHARDWARE, 2, "Tried to use a feature that requires hardware support." },
		{ FMOD_ERR_NET_CONNECT, 2, "Couldn't connect to the specified host." },
		{ FMOD_ERR_NET_SOCKET_ERROR, 2, "A socket error occurred. This is a catch-all for socket-related errors not listed elsewhere." },
		{ FMOD_ERR_NET_URL, 2, "The specified URL couldn't be resolved." },
		{ FMOD_ERR_NET_WOULD_BLOCK, 2, "Operation on a non-blocking socket could not complete immediately." },
		{ FMOD_ERR_NOTREADY, 2, "Operation could not be performed because specified Sound/DSP connection is not ready." },
		{ FMOD_ERR_OUTPUT_ALLOCATED, 2, "Error initializing output device, but more specifically, the output device is already in use and cannot be reused." },
		{ FMOD_ERR_OUTPUT_CREATEBUFFER, 2, "Error creating hardware sound buffer." },
		{ FMOD_ERR_OUTPUT_DRIVERCALL, 2, "A call to a standard soundcard driver failed, which could possibly mean a bug in the driver or resources were missing or exhausted." },
		{ FMOD_ERR_OUTPUT_FORMAT, 2, "Soundcard does not support the specified format." },
		{ FMOD_ERR_OUTPUT_INIT, 2, "Error initializing output device." },
		{ FMOD_ERR_OUTPUT_NODRIVERS, 2, "The output device has no drivers installed. If pre-init, FMOD_OUTPUT_NOSOUND is selected as the output mode. If post-init, the function just fails." },
		{ FMOD_ERR_PLUGIN, 2, "An unspecified error has been returned from a plugin." },
		{ FMOD_ERR_PLUGIN_MISSING, 2, "A requested output, dsp unit type or codec was not available." },
		{ FMOD_ERR_PLUGIN_RESOURCE, 2, "A resource that the plugin requires cannot be allocated or found. (ie the DLS file for MIDI playback)" },
		{ FMOD_ERR_PLUGIN_VERSION, 2, "A plugin was built with an unsupported SDK version." },
		{ FMOD_ERR_RECORD, 2, "" },
		{ FMOD_ERR_REVERB_CHANNELGROUP, 2, "" },
		{ FMOD_ERR_REVERB_INSTANCE, 2, "" },
		{ FMOD_ERR_SUBSOUNDS, 2, "The error occurred because the Sound referenced contains subsounds when it shouldn't have, or it doesn't contain subsounds when it should have. The operation may also not be able to be performed on a parent Sound." },
		{ FMOD_ERR_SUBSOUND_ALLOCATED, 2, "This subsound is already being used by another Sound, you cannot have more than one parent to a Sound. Null out the other parent's entry first." },
		{ FMOD_ERR_SUBSOUND_CANTMOVE, 2, "Shared subsounds cannot be replaced or moved from their parent stream, such as when the parent stream is an FSB file." },
		{ FMOD_ERR_TAGNOTFOUND, 2, "The specified tag could not be found or there are no tags." },
		{ FMOD_ERR_TOOMANYCHANNELS, 2, "The Sound created exceeds the allowable input channel count. This can be increased using the 'maxinputchannels' parameter in System::setSoftwareFormat." },
		{ FMOD_ERR_TRUNCATED, 2, "The retrieved string is too long to fit in the supplied buffer and has been truncated." },
		{ FMOD_ERR_UNIMPLEMENTED, 2, "Something in FMOD hasn't been implemented when it should be. Contact support." },
		{ FMOD_ERR_UNINITIALIZED, 2, "This command failed because System::init or System::setDriver was not called." },
		{ FMOD_ERR_UNSUPPORTED, 2, "A command issued was not supported by this object. Possibly a plugin without certain callbacks specified." },
		{ FMOD_ERR_VERSION, 2, "The version number of this file format is not supported." },
		{ FMOD_ERR_EVENT_ALREADY_LOADED, 2, "The specified bank has already been loaded." },
		{ FMOD_ERR_EVENT_LIVEUPDATE_BUSY, 2, "The live update connection failed due to the game already being connected." },
		{ FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH, 2, "The live update connection failed due to the game data being out of sync with the tool." },
		{ FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT, 2, "The live update connection timed out." },
		{ FMOD_ERR_EVENT_NOTFOUND, 2, "The requested event, parameter, bus or vca could not be found." },
		{ FMOD_ERR_STUDIO_UNINITIALIZED, 2, "The Studio::System object is not yet initialized." },
		{ FMOD_ERR_STUDIO_NOT_LOADED, 2, "The specified resource is not loaded, so it can't be unloaded." },
		{ FMOD_ERR_INVALID_STRING, 2, "An invalid string was passed to this function." },
		{ FMOD_ERR_ALREADY_LOCKED, 2, "The specified resource is already locked." },
		{ FMOD_ERR_NOT_LOCKED, 2, "The specified resource is not locked, so it can't be unlocked." },
		{ FMOD_ERR_RECORD_DISCONNECTED, 2, "The specified recording driver has been disconnected." },
		{ FMOD_ERR_TOOMANYSAMPLES, 2, "The length provided exceeds the allowable limit." }
	};

	for ( i = 0; i < arraylen( fmod_errors ); i++ ) {
		if ( result == fmod_errors[i].num ) {
			break;
		}
	}
	if ( i == arraylen( fmod_errors ) ) {
		Con_Printf( COLOR_RED "FMOD API Error: unknown error\n" );
		return;
	}

	switch ( fmod_errors[ i ].severity ) {
	case 2:
		N_Error( ERR_FATAL, "FMOD API Error: %s", fmod_errors[ i ].str );
		break;
	case 1:
		N_Error( ERR_DROP, "FMOD API Error: %s", fmod_errors[ i ].str );
		break;
	case 0:
		Con_Printf( COLOR_RED "FMOD API Error: %s\n", fmod_errors[ i ].str );
		break;
	};
}

void CSoundSource::Release( void )
{
	Stop();
	if ( m_pData ) {
		ERRCHECK( m_pData->releaseAllInstances() );
	}
	m_pData = NULL;
}

bool CSoundSource::Load( const char *npath, int64_t nTag )
{
	Con_Printf( "Loading sound source '%s'...\n", npath );

	N_strncpyz( m_szName, npath, sizeof( m_szName ) );

	// hash it so that if we try loading it
	// even if it's failed, we won't try loading
	// it again
	sndManager->AddSourceToHash( this );

	CSoundSystem::GetStudioSystem()->getEvent( npath, &m_pData );
	if ( !m_pData ) {
		Con_Printf( COLOR_YELLOW "WARNING: Error loading sound source. Event not found.\n" );
		return false;
	}

	m_nTag = nTag;

	return true;
}

FMOD::Studio::EventInstance *CSoundSource::AllocEvent( void )
{
	FMOD::Studio::EventInstance *pEvent;

	if ( !m_pData ) {
		return NULL;
	}

	ERRCHECK( m_pData->createInstance( &pEvent ) );

	return pEvent;
}

void CSoundSource::Play( bool bLooping, uint64_t nTimeOffset, bool bIsWorldSound )
{
	FMOD_STUDIO_PLAYBACK_STATE state;

	if ( !m_pData ) {
		return;
	}

	ERRCHECK( m_pData->createInstance( &m_pEmitter ) );
	ERRCHECK( m_pEmitter->getPlaybackState( &state ) );
	ERRCHECK( m_pEmitter->start() );

	if ( m_nTag == TAG_SFX ) {
		ERRCHECK( m_pEmitter->setVolume( snd_effectsVolume->f / 100.0f ) );
	} else if ( m_nTag == TAG_MUSIC ) {
		ERRCHECK( m_pEmitter->setVolume( snd_musicVolume->f / 100.0f ) );
	}

	{
		bool isSnapshot;
		m_pData->isSnapshot( &isSnapshot );
		if ( !isSnapshot && m_nTag != TAG_MUSIC && !bIsWorldSound ) {
			m_pEmitter->release();
			m_pEmitter = NULL;
		}
	}
}

void CSoundSource::Stop( void )
{
	FMOD_STUDIO_PLAYBACK_STATE state;

	if ( !m_pData || !m_pEmitter ) {
		return;
	}

	ERRCHECK( m_pEmitter->getPlaybackState( &state ) );

	switch ( state ) {
	case FMOD_STUDIO_PLAYBACK_PLAYING:
	case FMOD_STUDIO_PLAYBACK_STARTING: {
		if ( m_nTag == TAG_SFX ) {
			ERRCHECK( m_pEmitter->stop( FMOD_STUDIO_STOP_IMMEDIATE ) );
		} else if ( m_nTag == TAG_MUSIC ) {
			ERRCHECK( m_pEmitter->stop( FMOD_STUDIO_STOP_ALLOWFADEOUT ) );
		}
		break; }
	case FMOD_STUDIO_PLAYBACK_SUSTAINING:
	case FMOD_STUDIO_PLAYBACK_STOPPING:
		ERRCHECK( m_pEmitter->stop( FMOD_STUDIO_STOP_ALLOWFADEOUT ) );
		break;
	case FMOD_STUDIO_PLAYBACK_STOPPED:
		break;
	};

	ERRCHECK( m_pEmitter->release() );

	m_pEmitter = NULL;
}

bool CSoundSystem::LoadBank( const char *pName )
{
	CSoundBank *pBank;
	uint64_t slot;

	for ( slot = 0; slot < MAX_SOUND_BANKS; slot++ ) {
		if ( !m_szBanks[ slot ] ) {
			break;
		}
	}
	if ( slot >= MAX_SOUND_BANKS ) {
		N_Error( ERR_DROP, "CSoundSystem::LoadBank: too many sound banks!" );
	}

	pBank = (CSoundBank *)Hunk_Alloc( sizeof( *pBank ), h_low );
	if ( !pBank->Load( pName ) ) {
		return false;
	}

	m_szBanks[ slot ] = pBank;

	return true;
}

void CSoundSystem::AddSourceToHash( CSoundSource *pSource )
{
	nhandle_t hash;

	hash = Snd_HashFileName( pSource->GetName() );

	pSource->m_pNext = m_szSources[ hash ];
	m_szSources[ hash ] = pSource;
}

void CSoundSystem::ForceStop( void )
{
	for ( auto& it : m_szSources ) {
		if ( it ) {
			it->Stop();
		}
	}
	Snd_ClearLoopingTracks();
}

void CSoundSystem::SetParameter( const char *pName, float value )
{
	ERRCHECK( s_pStudioSystem->setParameterByName( pName, value, false ) );
}

static FMOD_RESULT fmod_debug_callback( FMOD_DEBUG_FLAGS flags, const char *file, int line, const char *func, const char *message )
{
	if ( flags & FMOD_DEBUG_LEVEL_ERROR ) {
		FS_Printf( fmodLogFile, "[%s:%s:%i] %s", file, func, line, message );
		N_Error( ERR_DROP, COLOR_RED "[FMOD API][%s:%s:%i] %s", file, func, line, message );
	} else if ( flags & FMOD_DEBUG_LEVEL_WARNING ) {
		FS_Printf( fmodLogFile, "[%s:%s:%i] %s", file, func, line, message );
		Con_Printf( COLOR_YELLOW "[FMOD API][%s:%s:%i] %s", file, func, line, message );
	} else {
		FS_Printf( fmodLogFile, "[%s:%s:%i] %s", file, func, line, message );
		Con_Printf( "[FMOD API][%s:%s:%i] %s", file, func, line, message );
	}

	return FMOD_OK;
}

void CSoundSystem::Init( void )
{
	int ret;
	FMOD_INITFLAGS flags;

	if ( snd_debug->i ) {
		Con_Printf( "Sound debug log enabled, creating logfile...\n" );

		fmodLogFile = FS_FOpenWrite( "Logs/fmod.log" );
		if ( fmodLogFile ) {
			Con_Printf( COLOR_YELLOW "WARNING: Error creating logfile!\n" );
		}

		ERRCHECK( FMOD::Debug_Initialize( FMOD_DEBUG_LEVEL_LOG | FMOD_DEBUG_LEVEL_ERROR | FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_TYPE_TRACE
			| FMOD_DEBUG_DISPLAY_THREAD, FMOD_DEBUG_MODE_CALLBACK, fmod_debug_callback, NULL ) );
	}

	flags = FMOD_INIT_CHANNEL_DISTANCEFILTER | FMOD_INIT_PROFILE_ENABLE | FMOD_INIT_THREAD_UNSAFE | FMOD_INIT_VOL0_BECOMES_VIRTUAL
		| FMOD_INIT_CHANNEL_LOWPASS;

	ERRCHECK( FMOD::Studio::System::create( &s_pStudioSystem ) );
	ERRCHECK( s_pStudioSystem->getCoreSystem( &s_pCoreSystem ) );

	// get default audio info
	memset( &m_AudioInfo, 0, sizeof( m_AudioInfo ) );
	ERRCHECK( s_pCoreSystem->setSoftwareFormat( 48000, (FMOD_SPEAKERMODE)snd_speakerMode->i, 0 ) );
	
	ERRCHECK( s_pCoreSystem->getSoftwareFormat( &m_AudioInfo.samplerate, &m_AudioInfo.audioMode, &m_AudioInfo.speakerCount ) );
	Cvar_SetIntegerValue( "snd_speakerMode", m_AudioInfo.audioMode );

	ERRCHECK( s_pCoreSystem->set3DSettings( 1.0f, DISTANCEFACTOR, 0.5f ) );
#ifdef _NOMAD_DEBUG
	ERRCHECK( s_pStudioSystem->initialize( snd_maxChannels->i, FMOD_STUDIO_INIT_LIVEUPDATE,
		flags, NULL ) );
#else
	ERRCHECK( s_pStudioSystem->initialize( snd_maxChannels->i, FMOD_STUDIO_INIT_NORMAL,
		flags, NULL ) );
#endif

	m_pStudioSystem = s_pStudioSystem;
	m_pSystem = s_pCoreSystem;

	{
		char **fileList;
		char szPath[ MAX_OSPATH ];
		uint64_t i, nFiles;

		fileList = FS_ListFiles( "soundbanks/", ".fsb", &nFiles );
		for ( i = 0; i < nFiles; i++ ) {
			N_strncpyz( szPath, COM_SkipPath( fileList[ i ] ), sizeof( szPath ) );
			COM_StripExtension( szPath, szPath, sizeof( szPath ) );
			LoadBank( szPath );
		}

		FS_FreeFileList( fileList );
	}

	ERRCHECK( s_pStudioSystem->getBus( "bus:/SFX", &m_pSFXBus ) );
	ERRCHECK( s_pStudioSystem->getBus( "bus:/Music", &m_pMusicBus ) );
}

void CSoundSystem::Shutdown( void )
{
	Snd_ClearLoopingTracks();

	ERRCHECK( m_pSFXBus->stopAllEvents( FMOD_STUDIO_STOP_IMMEDIATE ) );
	ERRCHECK( m_pMusicBus->stopAllEvents( FMOD_STUDIO_STOP_IMMEDIATE ) );

	m_szLoopingTracks.clear();
	for ( auto& it : m_szSources ) {
		if ( !it ) {
			continue;
		}
		it->Release();
	}
	for ( auto& it : m_szBanks ) {
		if ( !it ) {
			break;
		}
		it->Shutdown();
	}
	memset( m_szSources, 0, sizeof( m_szSources ) );
	memset( m_szBanks, 0, sizeof( m_szBanks ) );

	m_nSources = 0;

	ERRCHECK( s_pStudioSystem->release() );

	gi.soundRegistered = qfalse;
	gi.soundStarted = qfalse;

	s_SoundWorld.Shutdown();

	if ( fmodLogFile ) {
		FS_FClose( fmodLogFile );
	}

	Z_FreeTags( TAG_SFX );
	Z_FreeTags( TAG_MUSIC );

	Cmd_RemoveCommand( "snd.setvolume" );
	Cmd_RemoveCommand( "snd.toggle" );
	Cmd_RemoveCommand( "snd.updatevolume" );
	Cmd_RemoveCommand( "snd.list_files" );
	Cmd_RemoveCommand( "snd.clear_tracks" );
	Cmd_RemoveCommand( "snd.play_sfx" );
	Cmd_RemoveCommand( "snd.queue_track" );
 	Cmd_RemoveCommand( "snd.startup_level" );
	Cmd_RemoveCommand( "snd.unload_level" );
}

void CSoundSystem::Update( void )
{
	if ( snd_muteUnfocused->i ) {
		ERRCHECK( m_pSFXBus->setMute( !gw_active ) );
		ERRCHECK( m_pMusicBus->setMute( !gw_active ) );
	}
	if ( snd_musicOn->modified ) {
		ERRCHECK( m_pMusicBus->setMute( snd_musicOn->i ) );
		snd_musicOn->modified = qfalse;
	}
	if ( snd_effectsOn->modified ) {
		ERRCHECK( m_pSFXBus->setMute( snd_effectsOn->i ) );
		snd_effectsOn->modified = qfalse;
	}
	if ( snd_musicVolume->modified ) {
		ERRCHECK( m_pMusicBus->setVolume( snd_musicVolume->f / 100.0f ) );
		snd_musicVolume->modified = qfalse;
	}
	if ( snd_effectsVolume->modified ) {
		ERRCHECK( m_pSFXBus->setVolume( snd_effectsVolume->f / 100.0f ) );
		snd_effectsVolume->modified = qfalse;
	}
//	if ( gi.mapLoaded && g_paused->modified ) {
//		if ( g_paused->i ) {
//			Snd_PlaySfx( Snd_RegisterSfx( "snapshot:/PauseMenu" ) );
//		} else {
//			Snd_StopSfx( Snd_RegisterSfx( "snapshot:/PauseMenu" ) );
//		}
//	}

	ERRCHECK( s_pStudioSystem->update() );
	ERRCHECK( s_pCoreSystem->update() );
}

CSoundSource *CSoundSystem::LoadSound( const char *npath, int64_t nTag )
{
	CSoundSource *pSound;
	sfxHandle_t hash;

	hash = Snd_HashFileName( npath );

	//
	// check if we already have it loaded
	//
	for ( pSound = m_szSources[hash]; pSound; pSound = pSound->m_pNext ) {
		if ( !N_stricmp( npath, pSound->GetName() ) ) {
			return pSound;
		}
	}
	if ( strlen( npath ) >= MAX_NPATH ) {
		Con_Printf( "CSoundManager::InitSource: name '%s' too long\n", npath );
		return NULL;
	}
	if ( m_nSources == MAX_SOUND_SOURCES ) {
		N_Error( ERR_DROP, "CSoundManager::InitSource: MAX_SOUND_SOURCES hit" );
	}

	pSound = (CSoundSource *)Hunk_Alloc( sizeof( *pSound ), h_low );
	memset( pSound, 0, sizeof( *pSound ) );

	if ( !pSound->Load( npath, nTag ) ) {
		Con_Printf( COLOR_YELLOW "WARNING: failed to load sound file '%s'\n", npath );
		return NULL;
	}

	m_nSources++;

	return pSound;
}

void Snd_DisableSounds( void )
{
	sndManager->ForceStop();
}

void Snd_StopAll( void )
{
	sndManager->ForceStop();
}

void Snd_PlaySfx( sfxHandle_t sfx )
{
	if ( sfx == -1 || !snd_effectsOn->i ) {
		return;
	}
	CSoundSource *pSource = sndManager->GetSound( sfx );
	if ( !pSource ) {
		return;
	}
	pSource->Play();
}

void Snd_StopSfx( sfxHandle_t sfx )
{
	if ( sfx == -1 || !snd_effectsOn->i ) {
		return;
	}
	CSoundSource *pSource = sndManager->GetSound( sfx );
	if ( !pSource ) {
		return;
	}
	pSource->Stop();
}

static void Snd_Toggle_f( void )
{
	const char *var;
	const char *toggle;
	bool option;

	if ( Cmd_Argc() < 3 ) {
		Con_Printf( "usage: snd.toggle <sfx|music> <on|off, 1|0>\n" );
		return;
	}

	var = Cmd_Argv( 1 );
	toggle = Cmd_Argv( 2 );
	option = true;

	if ( ( toggle[0] == '1' && toggle[1] == '\0' ) || !N_stricmp( toggle, "on" ) ) {
		option = true;
	} else if ( ( toggle[0] == '0' && toggle[1] == '\0' ) || !N_stricmp( toggle, "off" ) ) {
		option = false;
	}

	if ( !N_stricmp( var, "sfx" ) ) {
		Cvar_Set( "snd_effectsOn", va( "%i", option ) );
	} else if ( !N_stricmp( var, "music" ) ) {
		Cvar_Set( "snd_musicOn", va( "%i", option ) );
	} else {
		Con_Printf( "snd.toggle: unknown parameter '%s', use either 'sfx' or 'music'\n", var );
		return;
	}
}

static void Snd_SetVolume_f( void )
{
	float vol;
	const char *change;

	if ( Cmd_Argc() < 3 ) {
		Con_Printf( "usage: snd.setvolume <sfx|music> <volume>\n" );
		return;
	}

	vol = N_atof( Cmd_Argv( 1 ) );
	vol = CLAMP( vol, 0.0f, 100.0f );

	change = Cmd_Argv( 2 );
	if ( !N_stricmp( change, "sfx" ) ) {
		if ( snd_effectsVolume->f != vol ) {
			Cvar_Set( "snd_effectsVolume", va("%f", vol) );
//            sndManager->UpdateParm( TAG_SFX );
		}
	}
	else if ( !N_stricmp( change, "music" ) ) {
		if ( snd_musicVolume->f != vol ) {
			Cvar_Set( "snd_musicVolume", va( "%f", vol ) );
//            sndManager->UpdateParm( TAG_MUSIC );
		}
	}
	else {
		Con_Printf( "snd.setvolume: unknown parameter '%s', use either 'sfx' or 'music'\n", change );
		return;
	}
}

static void Snd_UpdateVolume_f( void ) {
//    sndManager->UpdateParm( TAG_SFX );
//    sndManager->UpdateParm( TAG_MUSIC );
}

static void Snd_ListFiles_f( void )
{
	uint64_t i, numFiles;
	const CSoundSource *source;

	Con_Printf( "\n---------- Snd_ListFiles_f ----------\n" );
	Con_Printf( "                      --channels-- ---samplerate--- ----cache size----\n" );

	numFiles = 0;
	for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
		source = sndManager->GetSound( i );

		if ( !source ) {
			continue;
		}
		numFiles++;
//        Con_Printf( "%10s: %4i %4i %8lu\n", source->GetName(), source->GetInfo().channels, source->GetInfo().samplerate,
//            source->GetInfo().channels * source->GetInfo().frames );
	}
	Con_Printf( "Total sound files loaded: %lu\n", numFiles );
}

static void Snd_PlayTrack_f( void ) {
	sfxHandle_t hSfx;
	const char *music;

	if ( !snd_musicOn->i ) {
		Con_Printf( "music is disabled\n" );
		return;
	}

	music = Cmd_Argv( 1 );
	
	Snd_ClearLoopingTracks();
	if ( !*music ) {
		Con_Printf( "Clearing current track...\n" );
		Snd_ClearLoopingTracks();
	} else {
		hSfx = Com_GenerateHashValue( music, MAX_SOUND_SOURCES );

		if ( sndManager->GetSound( hSfx ) == NULL ) {
			Con_Printf( "invalid track '%s'\n", music );
			return;
		}

		Snd_AddLoopingTrack( hSfx );
	}
}

static void Snd_QueueTrack_f( void ) {
	sfxHandle_t hSfx;
	const char *music;

	if ( !snd_musicOn->i ) {
		Con_Printf( "music is disabled\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "usage: snd.queue_track <music>\n" );
		return;
	}

	music = Cmd_Argv( 1 );
	hSfx = Com_GenerateHashValue( music, MAX_SOUND_SOURCES );
	
	if ( sndManager->GetSound( hSfx ) == NULL ) {
		hSfx = Snd_RegisterSfx( music );
		if ( hSfx == -1 ) {
			Con_Printf( "invalid track '%s'\n", music );
			return;
		}
	}

	Snd_AddLoopingTrack( hSfx );
}

static void Snd_ClearTracks_f( void ) {
	Snd_ClearLoopingTracks();
}

static void Snd_PlaySfx_f( void ) {
	sfxHandle_t hSfx;
	const char *sound;

	if ( !snd_effectsOn->i ) {
		Con_Printf( "sound effects are disabled\n" );
		return;
	}

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "usage: snd.play_sfx <music>\n" );
		return;
	}

	sound = Cmd_Argv( 1 );
	hSfx = Com_GenerateHashValue( sound, MAX_SOUND_SOURCES );
	
	if ( sndManager->GetSound( hSfx ) == NULL ) {
		Con_Printf( "invalid sfx '%s'\n", sound );
		return;
	}

	Snd_PlaySfx( hSfx );
}

void Snd_Restart( void )
{
	sndManager->Shutdown();

	sndManager->Init();
}

void Snd_Shutdown( void )
{
	sndManager->Shutdown();
}

void Snd_Update( int msec )
{
	if ( gi.mapLoaded ) {
		s_SoundWorld.Update();
	}
	sndManager->Update();
}

sfxHandle_t Snd_RegisterTrack( const char *npath )
{
	CSoundSource *pSource;

	pSource = sndManager->LoadSound( npath, TAG_MUSIC );
	if ( !pSource ) {
		return -1;
	}

	return Snd_HashFileName( npath );
}

sfxHandle_t Snd_RegisterSfx( const char *npath )
{
	CSoundSource *pSource;

	pSource = sndManager->LoadSound( npath, TAG_SFX );
	if ( !pSource ) {
		return -1;
	}

	return Snd_HashFileName( npath );
}

void Snd_PlayWorldSfx( const vec3_t origin, sfxHandle_t hSfx )
{
}

void Snd_SetWorldListener( const vec3_t origin )
{
}

void Snd_ClearLoopingTracks( void )
{
	for ( auto& it : sndManager->m_szLoopingTracks ) {
		it->Stop();
	}
	sndManager->m_szLoopingTracks.clear();
}

void Snd_AddLoopingTrack( sfxHandle_t handle, uint64_t timeOffset )
{
	CSoundSource *track;

	if ( !snd_musicOn->i ) {
		return;
	}

	if ( handle == -1 ) {
		Con_Printf( COLOR_RED "Snd_AddLoopingTrack: invalid handle, ignoring call.\n" );
		return;
	}

	track = sndManager->GetSound( handle );
	if ( !track ) {
		Con_Printf( COLOR_RED "Snd_AddLoopingTrack: sound didn't load properly (check log for more details), ignoring call.\n" );
		return;
	}
	if ( eastl::find( sndManager->m_szLoopingTracks.cbegin(), sndManager->m_szLoopingTracks.cend(), track )
		!= sndManager->m_szLoopingTracks.cend() )
	{
		return;
	}

//    if ( Snd_IsFreebird( track ) ) {
//        Con_Printf( "Your honor, freebird is playing\n" );
//    }

	track->Play( true, timeOffset );
	sndManager->m_szLoopingTracks.emplace_back( track );
}

void Snd_Init( void )
{
	Con_Printf( "---------- Snd_Init ----------\n" );

	snd_effectsOn = Cvar_Get( "snd_effectsOn", "1", CVAR_SAVE );
	Cvar_CheckRange( snd_effectsOn, "0", "1", CVT_INT );
	Cvar_SetDescription( snd_effectsOn, "Toggles sound effects." );

	snd_musicOn = Cvar_Get( "snd_musicOn", "1", CVAR_SAVE );
	Cvar_CheckRange( snd_musicOn, "0", "1", CVT_INT );
	Cvar_SetDescription( snd_musicOn, "Toggles music." );

	snd_effectsVolume = Cvar_Get( "snd_effectsVolume", "50", CVAR_SAVE );
	Cvar_CheckRange( snd_effectsVolume, "0", "100", CVT_INT );
	Cvar_SetDescription( snd_effectsVolume, "Sets global sound effects volume." );

	snd_musicVolume = Cvar_Get( "snd_musicVolume", "80", CVAR_SAVE );
	Cvar_CheckRange( snd_musicVolume, "0", "100", CVT_INT );
	Cvar_SetDescription( snd_musicVolume, "Sets volume for music." );

	snd_masterVolume = Cvar_Get( "snd_masterVolume", "80", CVAR_SAVE );
	Cvar_CheckRange( snd_masterVolume, "0", "100", CVT_INT );
	Cvar_SetDescription( snd_masterVolume, "Sets the cap for sfx and music volume." );

	snd_maxChannels = Cvar_Get( "snd_maxChannels", "256", CVAR_SAVE );
	Cvar_CheckRange( snd_maxChannels, "24", "512", CVT_INT );
	Cvar_SetDescription( snd_maxChannels,
		"Sets the maximum amount of channels the engine can process at a time.\n"
		"Higher values will increase CPU load.\n"
	);

#ifdef _WIN32
	Com_StartupVariable( "s_noSound" );
	snd_noSound = Cvar_Get( "s_noSound", "1", CVAR_LATCH );
#else
	Com_StartupVariable( "s_noSound" );
	snd_noSound = Cvar_Get( "s_noSound", "0", CVAR_LATCH );
#endif

	snd_debug = Cvar_Get( "snd_debug", "0", CVAR_SAVE );
	Cvar_SetDescription( snd_debug, "Enables fmod and sound system debug logging" );

	Cvar_Get( "snd_specialFlag", "-1", CVAR_TEMP | CVAR_PROTECTED );

	snd_muteUnfocused = Cvar_Get( "snd_muteUnfocused", "1", CVAR_SAVE );
	Cvar_SetDescription( snd_muteUnfocused, "Toggles muting sounds when the game's window isn't focused." );

	snd_speakerMode = Cvar_Get( "snd_speakerMode", "0", CVAR_SAVE );
	Cvar_SetDescription( snd_speakerMode, "Sets the speaker mode used by fmod" );

	// init sound manager
	sndManager = (CSoundSystem *)Hunk_Alloc( sizeof( *sndManager ), h_low );
	sndManager->Init();

	Cmd_AddCommand( "snd.setvolume", Snd_SetVolume_f );
	Cmd_AddCommand( "snd.toggle", Snd_Toggle_f );
	Cmd_AddCommand( "snd.updatevolume", Snd_UpdateVolume_f );
	Cmd_AddCommand( "snd.list_files", Snd_ListFiles_f );
	Cmd_AddCommand( "snd.clear_tracks", Snd_ClearTracks_f );
	Cmd_AddCommand( "snd.play_sfx", Snd_PlaySfx_f );
	Cmd_AddCommand( "snd.queue_track", Snd_QueueTrack_f );
	Cmd_AddCommand( "snd.play_track", Snd_PlayTrack_f );

	Snd_RegisterTrack( "warcrimes_are_permitted.ogg" );

	gi.soundStarted = qtrue;
	gi.soundRegistered = qtrue;

	Con_Printf( "----------------------------------\n" );
}