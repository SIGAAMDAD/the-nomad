#include "module_public.h"
#include "module_debugger.h"

CDebugger *g_pDebugger;

void Module_DebuggerSetActive_f( void ) {
	const char *name;
	CModuleInfo *mod;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "usage: ml_debug.set_active <module>\n" );
		return;
	}

	name = Cmd_Argv( 1 );
	mod = g_pModuleLib->GetModule( name );
	if ( !mod ) {
		Con_Printf( "invalid module '%s'.\n", name );
		return;
	}
	
	g_pDebugger->m_pModule = mod;
	
	Cvar_Set( "ml_debugMode", "1" );
	Con_Printf( "Setting module '%s' to debug mode.\n", name );
}

static void Module_Debugger_PrintValue_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->PrintValue( Cmd_Argv( 1 ), g_pDebugger->m_pModule->m_pHandle->GetContext() );
}

static void Module_Debugger_PrintCallstack_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->PrintCallstack( g_pDebugger->m_pModule->m_pHandle->GetContext() );
}

static void Module_Debugger_AddBreakPoint_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdSetBreakPoint();
}

static void Module_Debugger_RemoveBreakPoint_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdRemoveBreakPoint();
}

static void Module_Debugger_ListGlobals_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->ListGlobalVariables( g_pDebugger->m_pModule->m_pHandle->GetContext() );
}

static void Module_Debugger_ListLocals_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->ListLocalVariables( g_pDebugger->m_pModule->m_pHandle->GetContext() );
}

static void Module_Debugger_ListBreakPoints_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->ListBreakPoints();
}

static void Module_Debugger_ListMemberProperties_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->ListMemberProperties( g_pDebugger->m_pModule->m_pHandle->GetContext() );
}

static void Module_Debugger_PrintHelp_f( void ) {
	g_pDebugger->PrintHelp();
}

static void Module_Debugger_Continue_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdContinue();
}

static void Module_Debugger_StepInto_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdStepInto();
}

static void Module_Debugger_StepOver_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdStepOver();
}

static void Module_Debugger_StepOut_f( void ) {
	if ( !g_pDebugger->m_pModule ) {
		Con_Printf( "no active debugging module.\n" );
		return;
	}
	g_pDebugger->CmdStepOut();
}

CDebugger::CDebugger() {
	if ( g_pDebugger ) {
		N_Error( ERR_FATAL, "only one debugger can exist!" );
	}

	m_Action = CONTINUE;
	m_pLastFunction = NULL;
	g_pDebugger = this;

	Cmd_AddCommand( "ml_debug.set_active", Module_DebuggerSetActive_f );
	Cmd_AddCommand( "ml_debug.print_help", Module_Debugger_PrintHelp_f );
	Cmd_AddCommand( "ml_debug.stacktrace", Module_Debugger_PrintCallstack_f );
	Cmd_AddCommand( "ml_debug.backtrace", Module_Debugger_PrintCallstack_f );
	Cmd_AddCommand( "ml_debug.clear_breakpoint", Module_Debugger_RemoveBreakPoint_f );
	Cmd_AddCommand( "ml_debug.set_breakpoint", Module_Debugger_AddBreakPoint_f );
	Cmd_AddCommand( "ml_debug.continue", Module_Debugger_Continue_f );
	Cmd_AddCommand( "ml_debug.step_into", Module_Debugger_StepInto_f );
	Cmd_AddCommand( "ml_debug.step_out", Module_Debugger_StepOut_f );
	Cmd_AddCommand( "ml_debug.step_over", Module_Debugger_StepOver_f );
}

CDebugger::~CDebugger() {
}

void CDebugger::CmdStepOut( void ) {
	m_Action = STEP_OUT;
	m_nLastCommandAtStackLevel = m_pModule->m_pHandle->GetContext()->GetCallstackSize();
}

void CDebugger::CmdContinue( void ) {
	m_Action = CONTINUE;
}

void CDebugger::CmdStepInto( void ) {
	m_Action = STEP_INTO;
}

void CDebugger::CmdStepOver( void ) {
	m_Action = STEP_OVER;
	m_nLastCommandAtStackLevel = g_pDebugger->m_pModule->m_pHandle->GetContext()->GetCallstackSize();
}

void CDebugger::CmdSetBreakPoint( void ) {
	const char *point, *line;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "usage: br <function>|<file>:<line>\n" );
		return;
	}

	point = Cmd_Argv( 1 );

	if ( ( line = strrchr( point, ':' ) ) != NULL ) {
		const int32_t nLine = atoi( line + 1 );
		char file[MAX_NPATH];

		N_strncpyz( file, point, sizeof( file ) );
		*const_cast<char *>( line ) = 0;
		
		AddFileBreakPoint( file, nLine );
	} else {
		AddFuncBreakPoint( point );
	}
}

void CDebugger::CmdRemoveBreakPoint( void ) {
	const char *point, *line;
	
	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "usage: clear <function>|<file>:<line>\n" );
		return;
	}
	
	point = Cmd_Argv( 1 );
	
	if ( !N_stricmp( point, "all" ) ) {
		m_BreakPoints.clear();
		Con_Printf( "Cleared all breakpoints.\n" );
	}
	else {
		if ( ( line = strrchr( point, ':' ) ) != NULL ) {
			const int32_t nLine = atoi( line + 1 );
			char file[MAX_NPATH];

			*const_cast<char *>( line ) = 0;

			for ( auto it = m_BreakPoints.begin(); it != m_BreakPoints.end(); it++ ) {
				if ( !N_strcmp( file, it->pName ) && nLine == it->nLine && !it->bIsFunc ) {
					Con_Printf( "Cleared breakpoint at %s:%i\n", file, nLine );
					return;
				}
			}
			Con_Printf( "No breakpoint set at %s:%i.\n", file, nLine );
		} else { // function
			for ( auto it = m_BreakPoints.begin(); it != m_BreakPoints.end(); it++ ) {
				if ( !N_strcmp( point, it->pName ) && it->bIsFunc ) {
					m_BreakPoints.erase( it );
					Con_Printf( "Cleared breakpoint at %s.\n", point );
					return;
				}
			}
			Con_Printf( "No breakpoint set at %s.\n", point );
		}
	}
}

const char *CDebugger::ToString( void *pValue, asUINT nTypeId, int32_t nExpandMembers, asIScriptEngine *pEngine )
{
	static char str[MAXPRINTMSG];
	
	if ( !pValue ) {
		N_strncpyz( str, "(null)", sizeof( str ) );
	}
	
	if ( !pEngine ) {
		pEngine = g_pModuleLib->GetScriptEngine();
	}
	
	switch ( nTypeId ) {
	case asTYPEID_VOID:
		N_strncpyz( str, "<void>", sizeof( str ) );
		break;
	case asTYPEID_BOOL:
		Com_snprintf( str, sizeof( str ), "%s", *(bool *)pValue ? "true" : "false" );
		return str;
	case asTYPEID_INT8:
		Com_snprintf( str, sizeof( str ), "%hi", *(int8_t *)pValue );
		return str;
	case asTYPEID_INT16:
		Com_snprintf( str, sizeof( str ), "%hi", *(int16_t *)pValue );
		return str;
	case asTYPEID_INT32:
		Com_snprintf( str, sizeof( str ), "%i", *(int32_t *)pValue );
		return str;
	case asTYPEID_INT64:
		Com_snprintf( str, sizeof( str ), "%li", *(int64_t *)pValue );
		return str;
	case asTYPEID_UINT8:
		Com_snprintf( str, sizeof( str ), "%hu", *(uint8_t *)pValue );
		return str;
	case asTYPEID_UINT16:
		Com_snprintf( str, sizeof( str ), "%hu", *(uint16_t *)pValue );
		return str;
	case asTYPEID_UINT32:
		Com_snprintf( str, sizeof( str ), "%u", *(uint32_t *)pValue );
		return str;
	case asTYPEID_UINT64:
		Com_snprintf( str, sizeof( str ), "%lu", *(uint64_t *)pValue );
		return str;
	case asTYPEID_FLOAT:
		Com_snprintf( str, sizeof( str ), "%f", *(float *)pValue );
		return str;
	case asTYPEID_DOUBLE:
		Com_snprintf( str, sizeof( str ), "%lf", *(double *)pValue );
		return str;
	case asTYPEID_MASK_OBJECT: {
		Com_snprintf( str, sizeof( str ), "%u", *(asUINT *)pValue );
		
		if ( pEngine ) {
			asITypeInfo *type = pEngine->GetTypeInfoById( nTypeId );
			for ( uint32_t n = type->GetEnumValueCount(); n-- > 0; ) {
				int32_t enumValue;
				const char *enumName;
				
				enumName = type->GetEnumValueByIndex( n, &enumValue );
				if ( enumValue == *(int32_t *)pValue ) {
					Com_snprintf( str, sizeof( str ), ", %s", enumName );
					break;
				}
			}
		}
		break; }
	case asTYPEID_SCRIPTOBJECT: {
		if ( nTypeId & asTYPEID_OBJHANDLE ) {
			pValue = *(void **)pValue;
		}
		
		asIScriptObject *pObject = (asIScriptObject *)pValue;
		
		Com_snprintf( str, sizeof( str ), "{%p}", pValue );
		
		if ( pObject && nExpandMembers ) {
			asITypeInfo *type = pObject->GetObjectType();
			for ( uint32_t n = 0; n < pObject->GetPropertyCount(); n++ ) {
				if ( n == 0 ) {
					N_strcat( str, sizeof( str ), " " );
				} else {
					N_strcat( str, sizeof( str ), ", " );
				}
				
				N_strcat( str, sizeof( str ), va( "%s = %s",
					type->GetPropertyDeclaration( n ), ToString( pObject->GetAddressOfProperty( n ),
					pObject->GetPropertyTypeId( n ), nExpandMembers - 1, type->GetEngine() ) ) );
			}
		}
		break; }
	default: {
		if ( nTypeId & asTYPEID_OBJHANDLE ) {
			pValue = *(void **)pValue;
		}
		
		if ( pEngine ) {
			asITypeInfo *type = pEngine->GetTypeInfoById( nTypeId );
			if ( type->GetFlags() & asOBJ_REF ) {
				Com_snprintf( str, sizeof( str ), "{%p}", pValue );
			}
			if ( pValue ) {
				auto it = m_ToStringCallbacks.find( type );
				if ( it == m_ToStringCallbacks.end() ) {
					if ( type->GetFlags() & asOBJ_TEMPLATE ) {
						asITypeInfo *templateType = pEngine->GetTypeInfoByName( type->GetName() );
						it = m_ToStringCallbacks.find( templateType );
					}
				}
				
				if ( it != m_ToStringCallbacks.end() ) {
					if ( type->GetFlags() & asOBJ_REF ) {
						N_strcat( str, sizeof( str ), " " );
					}
					
					N_strcat( str, sizeof( str ), va( "%s", it->second( pValue, nExpandMembers, this ) ) );
				}
			}
		} else {
			N_strcat( str, sizeof( str ), "(no engine?)" );
		}
		break; }
	};
	return str;
}

void CDebugger::RegisterToStringCallback(const asITypeInfo *pTypeInfo, ToStringCallback_t pCallback )
{
	if ( m_ToStringCallbacks.find( pTypeInfo ) == m_ToStringCallbacks.end() ) {
		m_ToStringCallbacks.insert( UtlHashMap<const asITypeInfo *, ToStringCallback_t>::value_type( pTypeInfo, pCallback ) );
	}
}

void CDebugger::LineCallback( asIScriptContext *pContext )
{
	const char *pFileName;
	int32_t nLine;
	
	if ( !pContext ) {
		AssertMsg( pContext, "invalid context!" );
		return;
	}
	
	// by default we ignore callbacks when the context is not active.
	// an application might override this to for example disconnect the
	// debugger as the execution finished
	if ( pContext->GetState() == asEXECUTION_ACTIVE ) {
		return;
	}
	
	switch ( m_Action ) {
	case CONTINUE: {
		if ( !CheckBreakPoint( pContext ) ) {
			return;
		}
		break; }
	case STEP_OVER: {
		if ( pContext->GetCallstackSize() > m_nLastCommandAtStackLevel ) {
			if ( !CheckBreakPoint( pContext ) ) {
				return;
			}
		}
		break; }
	case STEP_OUT: {
		if ( pContext->GetCallstackSize() >= m_nLastCommandAtStackLevel ) {
			if ( !CheckBreakPoint( pContext ) ) {
				return;
			}
		}
		break; }
	case STEP_INTO: {
		CheckBreakPoint( pContext );
		
		// always break, but we call CheckBreakPoint anyway
		// to tell the user when the breakpoint has been reached
		break; }
	default:
		N_Error( ERR_FATAL, "invalid debug state!" );
	};
	
	nLine = pContext->GetLineNumber( 0, 0, &pFileName );
	Con_Printf( "Breakpoint hit: %s:%i %s\n",
		( pFileName ? pFileName : "{unnamed}" ), nLine, pContext->GetFunction()->GetDeclaration() );
}

bool CDebugger::CheckBreakPoint( asIScriptContext *pContext )
{
	const char *pTemp;
	int32_t nLine;
	const char *r;
	size_t n;
	asIScriptFunction *pFunc;
	const char *pFileName;
	
	if ( !pContext ) {
		AssertMsg( pContext, "invalid context!" );
		return false;
	}
	
	// TODO: Should cache the break points in a function by checking which possible break points
	//       can be hit when entering a function. If there are no break points in the current function
	//       then there is no need to check every line.

	nLine = pContext->GetLineNumber( 0, 0, &pTemp );

	// Consider just filename, not the full path
	pFileName = pTemp ? pTemp : "";
	r = strrchr( pFileName, PATH_SEP ); 
	if ( r ) {
		pFileName = r;
	}

	// did we move into a new function?
	pFunc = pContext->GetFunction();
	if ( m_pLastFunction != pFunc ) {
		// check if any breakpoints need adjusting
		for ( n = 0; n < m_BreakPoints.size(); n++ ) {
			// we need to check for a breakpoint at entering the function
			if ( m_BreakPoints[n].bIsFunc ) {
				if ( !N_stricmp( m_BreakPoints[n].pName, pFunc->GetName() ) ) {
					Con_Printf( "Entering function '%s'. Transforming it into breakpoint\n", m_BreakPoints[n].pName );

					// Transform the function breakpoint into a file breakpoint
					m_BreakPoints[n].pName           = pFileName;
					m_BreakPoints[n].nLine           = nLine;
					m_BreakPoints[n].bIsFunc         = false;
					m_BreakPoints[n].bNeedsAdjusting = false;
				}
			}
			// Check if a given breakpoint fall on a line with code or else adjust it to the next line
			else if ( m_BreakPoints[n].bNeedsAdjusting &&
					!N_strcmp( m_BreakPoints[n].pName, pFileName ) )
			{
				const int32_t line = pFunc->FindNextLineWithCode( m_BreakPoints[n].nLine );
				if ( line >= 0 ) {
					m_BreakPoints[n].bNeedsAdjusting = false;
					if ( line != m_BreakPoints[n].nLine ) {
						Con_Printf( "Moving breakpoint %lu in file '%s' to next line with code at line %i\n",
							n, pFileName, line );

						// Move the breakpoint to the next line
						m_BreakPoints[n].nLine = line;
					}
				}
			}
		}
	}
	m_pLastFunction = pFunc;

	// Determine if there is a breakpoint at the current line
	for ( n = 0; n < m_BreakPoints.size(); n++ ) {
		// TODO: do case-less comparison for file name

		// Should we break?
		if ( !m_BreakPoints[n].bIsFunc && m_BreakPoints[n].nLine == nLine && !N_stricmp( m_BreakPoints[n].pName, pFileName ) ) {
			Con_Printf( "Reached break point %lu in file '%s' at line %i\n", n, pFileName, nLine );
			return true;
		}
	}

	return false;
}

void CDebugger::PrintValue( const char *pExpression, asIScriptContext *pContext )
{
	if ( !pContext ) {
		Con_Printf( "no active modules.\n" );
		return;
	}
	
	asUINT nLength;
	char scope[MAX_STRING_CHARS];
	char name[4096];
	char str[4096];
	asETokenClass t;
	
	asIScriptEngine *pEngine = pContext->GetEngine();
	
	memset( scope, 0, sizeof( scope ) );
	memset( name, 0, sizeof( name ) );
	memset( str, 0, sizeof( str ) );
	
	nLength = 0;
	N_strncpyz( str, pExpression, sizeof( str ) );
	t = pEngine->ParseToken( str, 0, &nLength );
	while ( t == asTC_IDENTIFIER || ( t == asTC_KEYWORD && nLength == 2 && ( *str == ':' && *( str + 1 ) == ':' ) ) ) {
		if ( t == asTC_KEYWORD ) {
			if ( !*scope && !*name ) {
				// global scope
				strcpy( scope, "::" );
			} else if ( ( *scope == ':' && *( scope + 1 ) == ':' ) ) {
				// namespace
				N_strncpyz( scope, name, sizeof( scope ) );
			} else {
				N_strcat( scope, sizeof( scope ), va( "::%s", name ) );
			}
			*name = 0;
		}
		else if ( t == asTC_IDENTIFIER ) {
			memcpy( name, str, nLength );
		}
		
		// skip the parsed token and get the next one
		const char *temp = &str[ nLength ];
		memcpy( str, temp, nLength );
		t = pEngine->ParseToken( str, 0, &nLength );
	}
	
	if ( *name ) {
		void *ptr;
		int typeId;
		asIScriptFunction *func;
		
		func = pContext->GetFunction();
		if ( !func ) {
			return;
		}
		
		if ( !*scope ) {
			for ( asUINT n = func->GetVarCount(); n-- > 0; ) {
				const char *varName = NULL;
				pContext->GetVar( n, 0, &varName, &typeId );
				
				if ( pContext->IsVarInScope( n ) && varName && N_strcmp( name, varName ) == 0 ) {
					ptr = pContext->GetAddressOfVar( n );
					break;
				}
			}
			
			if ( !ptr && func->GetObjectType() ) {
				if ( N_strcmp( name, "this" ) == 0 ) {
					ptr = pContext->GetThisPointer();
					typeId = pContext->GetThisTypeId();
				} else {
					asITypeInfo *type = pEngine->GetTypeInfoById( pContext->GetThisTypeId() );
					for ( asUINT n = 0; n < type->GetPropertyCount(); n++ ) {
						const char *propName = NULL;
						int offset = 0;
						bool isRef = false;
						int compositeOffset = 0;
						bool isCompositeIndirect = false;
						
						type->GetProperty( n, &propName, &typeId, 0, 0, &offset, &isRef, 0, &compositeOffset,
							&isCompositeIndirect );
						
						if ( N_strcmp( name, propName ) == 0 ) {
							ptr = (void *)( ( (asBYTE *)pContext->GetThisPointer() ) + compositeOffset );
							if ( isCompositeIndirect ) {
								ptr = *(void **)ptr;
							}
							
							ptr = (void *)( ( (asBYTE *)ptr ) + offset );
							if ( isRef ) {
								ptr = *(void **)ptr;
							}
							break;
						}
					}
				}
			}
		}
		
		// look for global variables
		if ( !ptr ) {
			if ( !*scope ) {
				// if no explicit scope was informed then use the namespace of the current function by default
				N_strncpyz( scope, func->GetNamespace(), sizeof( scope ) );
			} else if ( *scope == ':' && *( scope + 1 ) == ':' ) {
				// the global namespace will be empty
				*scope = 0;
			}
			
			asIScriptModule *mod = func->GetModule();
			if ( mod ) {
				for ( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ ) {
					const char *varName = 0, *nameSpace = 0;
					
					mod->GetGlobalVar( n, &varName, &nameSpace, &typeId );
					
					// check if name & namespace match
					if ( N_strcmp( name, varName ) == 0 && N_strcmp( scope, nameSpace ) == 0 ) {
						ptr = mod->GetAddressOfGlobalVar( n );
						break;
					}
				}
			}
		}
		
		if ( ptr ) {
			// TODO: If there is a . after the identifier, check for members
			// TODO: If there is a [ after the identifier try to call the 'opIndex(expr) const' method 
			if ( *str ) {
				Con_Printf( "invalid expression. Expression doesn't end after symbol.\n" );
			}
			else {
				Con_Printf( "%s\n", ToString( ptr, typeId, 3, pEngine ) );
			}
		}
		else {
			// TODO: allow user to set if members should be expanded
			// expanded members by default to 3 recursive levels only
			Con_Printf( "invalid expression. No matching symbol.\n" );
		}
	}
	else {
		Con_Printf( "invalid expression. Expected identifier.\n" );
	}
}

void CDebugger::ListBreakPoints( void ) const
{
	Con_Printf( "BreakPoints: %lu\n", m_BreakPoints.size() );
	for ( size_t i = 0; i < m_BreakPoints.size(); i++ ) {
		if ( m_BreakPoints[i].bIsFunc ) {
			Con_Printf( "\t%lu - %s\n", i, m_BreakPoints[i].pName );
		} else {
			Con_Printf( "\t%lu - %s:%i\n", i, m_BreakPoints[i].pName, m_BreakPoints[i].nLine );
		}
	}
}

void CDebugger::ListMemberProperties( asIScriptContext *pContext )
{
	if ( !pContext ) {
		Con_Printf( "no module active.\n" );
		return;
	}
	
	void *ptr = pContext->GetThisPointer();
	if ( ptr ) {
		Con_Printf( "(0x%04lx) this: %s\n", (uintptr_t)ptr, ToString( ptr, pContext->GetThisTypeId(), 3, pContext->GetEngine() ) );
	}
}

void CDebugger::ListLocalVariables( asIScriptContext *pContext )
{
	if ( !pContext ) {
		Con_Printf( "no module active.\n" );
		return;
	}
	
	int typeId;
	const char *name;
	asIScriptFunction *func;
	
	func = pContext->GetFunction();
	if ( !func ) {
		return;
	}
	
	for ( asUINT n = 0; n < func->GetVarCount(); n++ ) {
		// TODO: option to view temporary variables
		const char *name;
		
		func->GetVar( n, &name );
		if ( !name || !*name ) {
			continue;
		}
		
		if ( pContext->IsVarInScope( n ) ) {
			pContext->GetVar( n, 0, 0, &typeId );
			Con_Printf( "%s = %s\n", func->GetVarDecl( n ), ToString( pContext->GetAddressOfVar( n ), typeId, 3, pContext->GetEngine() ) );
		}
	}
}

void CDebugger::ListGlobalVariables( asIScriptContext *pContext )
{
	if ( !pContext ) {
		Con_Printf( "no active module.\n" );
		return;
	}
	
	asIScriptFunction *func = pContext->GetFunction();
	asIScriptModule *mod = func->GetModule();
	
	if ( !func || !mod ) {
		return;
	}
	
	for ( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ ) {
		int typeId = 0;
		mod->GetGlobalVar( n, 0, 0, &typeId );
		
		// same TODO as the user member expansion
		Con_Printf( "%s = %s\n", mod->GetGlobalVarDeclaration( n ),
			ToString( mod->GetAddressOfGlobalVar( n ), typeId, 3, pContext->GetEngine() ) );
	}
}

void CDebugger::PrintCallstack( asIScriptContext *pContext )
{
	if ( !pContext ) {
		Con_Printf( "no module active.\n" );
		return;
	}
	
	const char *pFileName;
	int32_t nLine;
	 
	pFileName = NULL;
	nLine = 0;
	
	for ( asUINT n = 0; n < pContext->GetCallstackSize(); n++ ) {
	 	nLine = pContext->GetLineNumber( n, 0, &pFileName );
	 	
	 	Con_Printf( "[%u] %s/%s:%i - %s\n", n, pContext->GetFunction( n )->GetModuleName(), ( pFileName ? pFileName : "{unnamed}" ), nLine,
			pContext->GetFunction( n )->GetDeclaration() );
	}
}

void CDebugger::AddFuncBreakPoint( const char *pFuncName )
{
	UtlString func = pFuncName;
	
	const size_t b = func.find_first_not_of( " \t" );
	const size_t e = func.find_last_not_of( " \t" );
	const UtlString actual = func.substr( b, e != eastl::string::npos ? e - b + 1 : eastl::string::npos );
	
	Con_Printf( "Set breakpoint %lu at %s\n", m_BreakPoints.size(), actual.c_str() );
	
	m_BreakPoints.emplace_back( BreakPoint( actual.c_str(), 0, true ) );
}

void CDebugger::AddFileBreakPoint( const char *pFileName, int32_t nLine )
{
	UtlString path;
	
	N_strncpyz( path.data(), COM_SkipPath( const_cast<char *>( pFileName ) ), MAX_NPATH );
	
	const size_t b = path.find_first_not_of( " \t" );
	const size_t e = path.find_last_not_of( " \t" );
	const UtlString actual = path.substr( b, e != eastl::string::npos ? e - b + 1 : eastl::string::npos );
	
	Con_Printf( "Set breakpoint %lu at %s:%i\n", m_BreakPoints.size(), actual.c_str(), nLine );
	
	m_BreakPoints.emplace_back( BreakPoint( actual.c_str(), nLine, false ) );
}

void CDebugger::PrintHelp( void ) const {
	Con_Printf( "module debugger commands:\n"
				" cont        continue execution\n"
				" in          step into\n"
				" next        execute the next line\n"
				" out         step out\n"
				" br|break    set a breakpoint\n"
				" clearbr     remote a breakpoint\n"
				" print       print value of a variable\n"
				" bt|trace    print a stacktrace\n"
				" cancel      abort execution\n"
				" listglobals list all global variables\n"
				" listlocals  list all local variables\n"
				" gcstats     print garbage collection statistics\n"
				" help        print this help text\n"
	);
}
