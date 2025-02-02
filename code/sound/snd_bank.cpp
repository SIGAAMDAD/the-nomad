#include "snd_local.h"

FMOD::Studio::EventDescription *CSoundBank::GetEvent( const char *pName )
{
	FMOD::Studio::EventDescription *pEvent;
	int i, recieved;
	char szPath[ MAX_NPATH ];

	for ( i = 0; i < m_nEventCount; i++ ) {
		pEvent = m_pEventList[ i ];

		ERRCHECK( pEvent->getPath( szPath, sizeof( szPath ) - 1, &recieved ) );
		if ( !N_stricmp( pName, szPath ) ) {
			return pEvent;
		}
	}

	return NULL;
}

bool CSoundBank::Load( const char *npath )
{
	char *pBuffer;
	uint64_t nLength;
	int i, recieved;
	char szPath[ MAX_NPATH ];

	Con_Printf( "Loading sound bank file \"%s\"...\n", npath );

	N_strncpyz( m_szName, npath, sizeof( m_szName ) );

	Com_snprintf( szPath, sizeof( szPath ) - 1, "soundbanks/%s.fsb", npath );
	nLength = FS_LoadFile( szPath, (void **)&pBuffer );
	if ( !nLength || !pBuffer ) {
		Con_Printf( COLOR_RED "Error loading sound bank file \"soundbanks/%s.fsb\".\n", npath );
		return false;
	}

	ERRCHECK( CSoundSystem::GetStudioSystem()->loadBankMemory( pBuffer, nLength, FMOD_STUDIO_LOAD_MEMORY,
		FMOD_STUDIO_LOAD_BANK_NORMAL, &m_pBank ) );
	
	FS_FreeFile( pBuffer );

	Com_snprintf( szPath, sizeof( szPath ) - 1, "soundbanks/%s.fsb.strings", npath );
	nLength = FS_LoadFile( szPath, (void **)&pBuffer );
	if ( !nLength || !pBuffer ) {
		Con_Printf( COLOR_RED "Error loading sound bank strings file \"soundbanks/%s.fsb.strings\".\n", npath );
		m_pBank->unload();
		m_pBank = NULL;
		return false;
	}

	ERRCHECK( CSoundSystem::GetStudioSystem()->loadBankMemory( pBuffer, nLength, FMOD_STUDIO_LOAD_MEMORY,
		FMOD_STUDIO_LOAD_BANK_NORMAL, &m_pStrings ) );

	FS_FreeFile( pBuffer );

	ERRCHECK( m_pBank->getEventCount( &m_nEventCount ) );

/*
	ERRCHECK( m_pBank->getEventList( m_pEventList, m_nEventCount, &m_nEventCount ) );
	ERRCHECK( m_pBank->loadSampleData() );

	Con_Printf( "Loaded FMOD Bank File '%s':\n", npath );
	for ( i = 0; i < m_nEventCount; i++ ) {
		m_pEventList[ i ]->getPath( szPath, sizeof( szPath ) - 1, &recieved );
		Con_Printf( "- %s\n", szPath );
	}
*/

	return true;
}

void CSoundBank::Shutdown( void )
{
	m_pEventList = NULL;
	m_nEventCount = 0;

	if ( m_pBank ) {
		m_pBank->unloadSampleData();
		m_pBank->unload();
	}
	if ( m_pStrings ) {
		m_pStrings->unload();
	}
}