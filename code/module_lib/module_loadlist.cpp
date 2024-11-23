#include "module_loadlist.h"

CModuleLoadList *CModuleLoadList::g_pLoadList;

void CModuleLoadList::Init( const CModuleInfo *pModList, uint64_t nModCount )
{
	g_pLoadList = (CModuleLoadList *)Hunk_Alloc( sizeof( *g_pLoadList ), h_high );

	g_pLoadList->Load( pModList, nModCount );
	g_pLoadList->Resort();
}

void CModuleLoadList::Load( const CModuleInfo *pModList, uint64_t nModCount )
{
	uint32_t i, j;

	m_nModCount = nModCount;
	m_pModList = (module_t *)Hunk_Alloc( sizeof( *m_pModList ) * m_nModCount, h_high );

	// check for required modules
	for ( i = 0; i < m_nModCount; i++ ) {
		if ( !N_strcmp( m_pModList[i].szID, "nomadmain" ) || !N_stricmp( m_pModList[i].szID, "gameui" ) ) {
			m_pModList[i].flags |= ( MODULE_FLAG_ACTIVE | MODULE_FLAG_VALID | MODULE_FLAG_REQUIRED );
		}
	}

	for ( i = 0; i < m_nModCount; i++ ) {
		N_strncpyz( m_pModList[i].szName, pModList[i].m_szName, sizeof( m_pModList[i].szName ) );
		N_strncpyz( m_pModList[i].szID, pModList[i].m_szId, sizeof( m_pModList[i].szID ) );
		if ( !pModList[i].m_pHandle->IsValid() ) {
			m_pModList[i].flags &= ~MODULE_FLAG_VALID;
		} else {
			m_pModList[i].flags |= MODULE_FLAG_ACTIVE;
		}

		if ( pModList[i].m_GameVersion.m_nVersionMajor != _NOMAD_VERSION_MAJOR
			|| pModList[i].m_GameVersion.m_nVersionUpdate != _NOMAD_VERSION_UPDATE
			|| pModList[i].m_GameVersion.m_nVersionMajor != _NOMAD_VERSION_PATCH )
		{
			m_pModList[i].flags |= MODULE_FLAG_BADVERSION;
		}

		m_pModList[i].modVersion.m_nVersionMajor = pModList[i].m_nModVersionMajor;
		m_pModList[i].modVersion.m_nVersionUpdate = pModList[i].m_nModVersionUpdate;
		m_pModList[i].modVersion.m_nVersionPatch = pModList[i].m_nModVersionPatch;

		m_pModList[i].gameVersion = pModList[i].m_GameVersion;

		m_pModList[i].numDependencies = pModList[i].m_nDependencies;
		m_pModList[i].dependencies = (uint32_t *)Hunk_Alloc( sizeof( *m_pModList[i].dependencies ) * pModList[i].m_nDependencies, h_high );
	}
	for ( i = 0; i < m_nModCount; i++ ) {
		for ( j = 0; j < m_pModList[i].numDependencies; j++ ) {
			m_pModList[i].dependencies[j] = GetModuleIndex( FindModule( pModList[i].m_pDependencies[j].c_str() ) );
		}
	}

	if ( !FS_FileExists( CACHE_DIR "/loadlist.cfg" ) ) {
//		eastl::sort( m_pModList, m_pModList + m_nModCount );
		return; // doesn't exist yet
	}
	
	Con_Printf( "...Got %u modules\n", m_nModCount );
}

void CModuleLoadList::Resort( void )
{
	uint32_t i, j;
	module_t *mod;
	
	//
	// the first pass, the ensure mods are placed in proper order,
	// meaning dependencies are loaded first
	//
	Con_DPrintf( "reordering load list...\n" );
	for ( i = 0; i < m_nModCount; i++ ) {
		m_pModList[i].flags |= ( MODULE_FLAG_ALL_DEPS_LOADED | MODULE_FLAG_ALL_DEPS_ACTIVE );
		for ( j = 0; j < m_pModList[i].numDependencies; j++ ) {
			mod = &m_pModList[ m_pModList[i].dependencies[i] ];
			if ( mod == NULL ) {
				continue; // already dealt with in the firstpass
			}
			if ( IsLoadedBefore( &m_pModList[i], mod ) ) {
				eastl::swap( m_pModList[i], *mod );
			}
		}
		for ( j = 0; j < m_nModCount; j++ ) {
			if ( &m_pModList[i] == &m_pModList[j] ) {
				continue;
			}
			if ( IsDependedOn( &m_pModList[j], &m_pModList[i] ) && IsLoadedAfter( &m_pModList[i], &m_pModList[j] ) ) {
				Con_DPrintf( "...module \"%s\" is a dependency of \"%s\", resorting\n", m_pModList[i].szName, m_pModList[j].szName );
				eastl::swap( m_pModList[i], m_pModList[j] );
			}
		}
	}
	
	//
	// the second pass, to ensure all mods are loaded properly
	//
	Con_DPrintf( "checking validity of modules...\n" );
	for ( i = 0; i < m_nModCount; i++ ) {
		m_pModList[i].flags |= ( MODULE_FLAG_ALL_DEPS_LOADED | MODULE_FLAG_ALL_DEPS_ACTIVE );
		
		// check if all the dependencies are properly loaded
		for ( j = 0; j < m_pModList[i].numDependencies; j++ ) {
			mod = &m_pModList[ m_pModList[i].dependencies[i] ];
			if ( !mod ) {
				continue;
			}
			if ( mod == NULL ) {
				Con_DPrintf( "...module \"%s\" doesn't have all it's dependencies loaded (wasn't in load list)\n",
					m_pModList[i].szName );
				m_pModList[i].flags &= ~MODULE_FLAG_ALL_DEPS_LOADED;
				break;
			}
			else if ( !( mod->flags & MODULE_FLAG_VALID ) || !( mod->flags & MODULE_FLAG_ALL_DEPS_LOADED ) ) {
				Con_DPrintf( "...module \"%s\" doesn't have all it's dependencies loaded (didn't load properly)\n",
					m_pModList[i].szName );
				m_pModList[i].flags &= ~MODULE_FLAG_ALL_DEPS_LOADED;
				break;
			}
			else if ( !( mod->flags & MODULE_FLAG_ACTIVE ) || !( mod->flags & MODULE_FLAG_ALL_DEPS_ACTIVE ) ) {
				Con_DPrintf( "...module \"%s\" doesn't have all it's dependencies loaded (isn't toggled on)\n",
					m_pModList[i].szName );
				m_pModList[i].flags &= ~MODULE_FLAG_ALL_DEPS_ACTIVE;
				break;
			}
			else {
				// if a module is active, force all it's dependencies on as well
				if ( m_pModList[i].flags & MODULE_FLAG_ACTIVE ) {
					mod->flags |= MODULE_FLAG_ACTIVE;
				}
			}
		}
		
		// if the module isn't loaded properly, don't allow activation
		if ( !( m_pModList[i].flags & MODULE_FLAG_VALID ) ) {
			Con_DPrintf( "...module \"%s\" wasn't loaded propery, forcefully disabling\n",
				m_pModList[i].szName );
			m_pModList[i].flags &= ~MODULE_FLAG_ACTIVE;
			
			// disable any depended modules
			for ( j = 0; j < m_nModCount; j++ ) {
				if ( IsDependedOn( &m_pModList[i], &m_pModList[j] ) ) {
					m_pModList[i].flags &= ~MODULE_FLAG_ALL_DEPS_LOADED;
				}
			}
		}
		
		// if the module isn't active, don't allow any dependent modules
		if ( !( m_pModList[i].flags & MODULE_FLAG_ACTIVE ) ) {
			Con_DPrintf( "...module \"%s\" hasn't been activated\n", m_pModList[i].szName );
			
			// disable any depended modules
			for ( j = 0; j < m_nModCount; j++ ) {
				if ( IsDependedOn( &m_pModList[j], &m_pModList[i] ) ) {
					m_pModList[i].flags &= ~( MODULE_FLAG_ALL_DEPS_ACTIVE );
				}
			}
		} else {
			for ( j = 0; j < m_pModList[i].numDependencies; j++ ) {
				mod = &m_pModList[ m_pModList[i].dependencies[i] ];
				if ( !mod ) {
					continue;
				}
				if ( mod->flags & MODULE_FLAG_ALL_DEPS_ACTIVE ) {
					m_pModList[i].flags |= MODULE_FLAG_ALL_DEPS_ACTIVE;
				} else {
					m_pModList[i].flags &= ~MODULE_FLAG_ALL_DEPS_ACTIVE;
				}
			}
		}
		
		// if we don't have all the dependencies, don't allow activation
		if ( !( m_pModList[i].flags & MODULE_FLAG_ALL_DEPS_LOADED ) ) {
			Con_DPrintf( "...module \"%s\" doesn't have all it's dependencies loaded\n", m_pModList[i].szName );
			m_pModList[i].flags &= ~( MODULE_FLAG_ACTIVE | MODULE_FLAG_VALID );
		}
		if ( !( m_pModList[i].flags & MODULE_FLAG_ALL_DEPS_ACTIVE ) ) {
			Con_DPrintf( "...module \"%s\" doesn't have all it's dependencies activated\n", m_pModList[i].szName );
			m_pModList[i].flags &= ~( MODULE_FLAG_ACTIVE );
		}
	}
}

const char *GetFlagStrings( uint32_t flags )
{
	static char str[1024];
	
	str[0] = '\0';
	
	if ( flags & MODULE_FLAG_VALID ) {
		N_strcat( str, sizeof( str ) - 1, " Valid" );
	}
	if ( flags & MODULE_FLAG_ACTIVE ) {
		strcat( str, " Active" );
	} else {
		strcat( str, " Inactive" );
	}
	if ( flags & MODULE_FLAG_ALL_DEPS_LOADED ) {
		strcat( str, " AllDepsLoaded" );
	}
	if ( flags & MODULE_FLAG_ALL_DEPS_ACTIVE ) {
		strcat( str, " AllDepsActive" );
	}
	
	return str;
}

void CModuleLoadList::PrintList_f( void ) const
{
	uint64_t i;
	uint32_t j;
	
	Con_DPrintf( "module load list:\n" );
	for ( i = 0; i < m_nModCount; i++ ) {
		Con_DPrintf( "%-4lu: %s (%s) -> 0x%lx\n", i, m_pModList[i].szName, m_pModList[i].szID, (uintptr_t)(void *)&m_pModList[i] );
		Con_DPrintf( "\tflags:%s\n", GetFlagStrings( m_pModList[i].flags ) );
		Con_DPrintf( "\t%u dependencies:\n", m_pModList[i].numDependencies );
		for ( j = 0; j < m_pModList[i].numDependencies; j++ ) {
			Con_DPrintf( "\t  %s\n", m_pModList[i].szID );
		}
	}
}