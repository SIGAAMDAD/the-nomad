#include "module_loadlist.h"

bool CModuleLoadList::Load( const CModuleInfo *pModList, uint64_t nModCount )
{
	char *b;
	uint64_t nLength;
	int i, j;
	const char **text;
	const char *text_p;
	const char *tok;
	char *modName;
	char **loadList;
	uint64_t loadIndex;

	m_nModCount = nModCount;

	m_pModList = (module_t *)Hunk_Alloc( sizeof( *m_pModList ) * m_nModCount, h_high );

	// check for required modules
	for ( i = 0; i < m_nModCount; i++ ) {
		if ( !N_strcmp( m_pModList[i].szID, "nomadmain" ) || !N_stricmp( m_pModList[i].szID, "gameui" ) ) {
			m_pModList[i].flags |= ( MODULE_FLAG_ACTIVE | MODULE_FLAG_VALID );
		}
	}

	nLength = FS_LoadFile( CACHE_DIR "/loadlist.cfg", (void **)&b );
	if ( !nLength || !b ) {
		eastl::sort( m_pModList, m_pModList + m_nModCount );
		return; // doesn't exist yet
	}

	text_p = b;
	text = (const char **)&text_p;

	loadIndex = 0;
	while ( 1 ) {
		tok = COM_ParseExt( text, qfalse );
		if ( !tok[0] ) {
			COM_ParseError( "unexpected end of load list file" );
			break;
		}
		for ( i = 0; i < m_nModuleCount; i++ ) {
			if ( !N_stricmp( m_pLoadList[ i ].m_szName, tok ) ) {
				m_pModList[ loadIndex ].info = &m_pLoadList[ i ];
			}
		}
		if ( !m_pModList[ loadIndex ].info ) {
			COM_ParseError( "invalid module in load list '%s'", tok );
			break;
		}
		N_strncpyz( m_pModList[ loadIndex ].info->m_szName, tok, sizeof( m_pModList[ loadIndex ].info->m_szName ) );

		tok = COM_ParseExt( text, qfalse );
		if ( !tok[0] ) {
			COM_ParseError( "missing parameter for 'valid' in module load list" );
		}
		m_pModList[ loadIndex ].valid = atoi( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( !tok[0] ) {
			COM_ParseError( "missing parameter for 'active' in module load list" );
			goto __error;
		}
		m_pModList[ loadIndex ].active = atoi( tok );

		m_pModList[ loadIndex ].bootIndex = loadIndex;

		if ( !m_pModList[ loadIndex ].active && IsRequiredModule( m_pModList[loadIndex].info->m_szName ) ) {
			m_pModList[ loadIndex ].active = qtrue; // force on
		}

		loadIndex++;

	__error:
		break;
	}

	FS_FreeFile( b );

	for ( i = 0; i < m_nModuleCount; i++ ) {
		m_pModList[i].valid = m_pLoadList[i].m_pHandle->IsValid();
		m_pModList[i].isRequired = N_streq( m_pModList[i].info->m_szName, "nomadmain" ) || N_streq( m_pModList[i].info->m_szName, "gameui" );
		m_pModList[i].numDependencies = m_pLoadList[i].m_nDependencies;

		// check if we have any dependencies that either don't exist or aren't properly loaded
		for ( j = 0; j < m_pLoadList[i].m_nDependencies; j++ ) {
			const CModuleInfo *dep = GetModule( m_pLoadList[i].m_pDependencies[j].c_str() );

			if ( !dep || !dep->m_pHandle->IsValid() ) {
				m_pModList[i].valid = qfalse;
			}
		}
	}
	
	// reorder
//	for ( i = 0; i < m_nModuleCount; i++ ) {
//		module_t m = m_pModList[i];
//		m_pModList[i] = m_pModList[ m_pModList[i].bootIndex ];
//		m_pModList[ m_pModList[i].bootIndex ] = m;
//	}

//	eastl::sort( m_pModList, m_pModList + m_nModuleCount );

	// check for missing dependencies
	for ( i = 0; i < m_nModuleCount; i++ ) {
		bool done = false;
		m_pModList[i].allDepsActive = qtrue;
		for ( j = 0; j < m_pModList[i].info->m_nDependencies; j++ ) {
			for ( j = 0; j < m_nModuleCount; j++ ) {
				if ( N_strcmp( m_pModList[j].info->m_szName, m_pLoadList[i].m_pDependencies[j].c_str() ) == 0 ) {
					if ( !m_pModList[j].info->m_pHandle->IsValid() ) {
						m_pModList[i].allDepsActive = qfalse;
						done = true;
						Con_Printf( COLOR_YELLOW "WARNING: module \"%s\" missing dependency \"%s\"\n",
							m_pLoadList[i].m_pDependencies[j].c_str(), m_pModList[i].info->m_szName );
						break;
					}
				}
			}
			if ( done ) {
				break;
			}
		}
		m_pModList[i].info = &m_pLoadList[i];
		m_pModList[i].valid = m_pModList[i].allDepsActive;
	}

	Con_Printf( "...Got %lu modules\n", m_nModuleCount );
	
	return true;
}

void CModuleLoadList::Resort( void )
{
	uint64_t i, j;
	module_t *mod;
	
	//
	// the first pass, the ensure mods are placed in proper order,
	// meaning dependencies are loaded first
	//
	Con_DPrintf( "reordering load list...\n" );
	for ( i = 0; i < m_nModCount; i++ ) {
		m_pModList[i].flags |= ( MODULE_FLAG_ALL_DEPS_LOADED | MODULE_FLAG_ALL_DEPS_ACTIVE );
		for ( const auto& dep : m_pModList[i].dependencies ) {
			mod = FindModule( dep );
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
		for ( const auto& dep : m_pModList[i].dependencies ) {
			mod = FindModule( dep );
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
	        for ( const auto& dep : m_pModList[i].dependencies ) {
	            mod = FindModule( dep );
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
	
	Con_DPrintf( "module load list:\n" );
	for ( i = 0; i < m_nModCount; i++ ) {
		Con_DPrintf( "%-4lu: %s (%s) -> 0x%lx\n", i, m_pModList[i].szName, m_pModList[i].szID, (uintptr_t)(void *)&m_pModList[i] );
		Con_DPrintf( "\tflags:%s\n", GetFlagStrings( m_pModList[i].flags ) );
		Con_DPrintf( "\t%lu dependencies:\n", m_pModList[i].dependencies.size() );
		for ( const auto& it : m_pModList[i].dependencies ) {
			printf( "\t  %s\n", it.c_str() );
		}
	}
}