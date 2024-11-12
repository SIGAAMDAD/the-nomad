/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "module_public.h"
#include "module_alloc.h"
#include "module_handle.h"
#include "angelscript/as_bytecode.h"
#include "module_funcdefs.hpp"
#include "module_debugger.h"
#include "angelscript/as_scriptobject.h"
#include "scriptpreprocessor.h"
#include "../game/g_world.h"

const moduleFunc_t funcDefs[NumFuncs] = {
	{ "ModuleOnInit", ModuleInit, 0, qtrue, qfalse },
	{ "ModuleOnShutdown", ModuleShutdown, 0, qtrue, qfalse },
	{ "ModuleOnConsoleCommand", ModuleCommandLine, 0, qfalse, qfalse },
	{ "ModuleDrawConfiguration", ModuleDrawConfiguration, 0, qfalse, qtrue },
	{ "ModuleSaveConfiguration", ModuleSaveConfiguration, 0, qfalse, qtrue },
	{ "ModuleOnKeyEvent", ModuleOnKeyEvent, 2, qfalse, qfalse },
	{ "ModuleOnMouseEvent", ModuleOnMouseEvent, 2, qfalse, qfalse },
	{ "ModuleOnLevelStart", ModuleOnLevelStart, 0, qfalse, qfalse },
	{ "ModuleOnLevelEnd", ModuleOnLevelEnd, 0, qfalse, qfalse },
	{ "ModuleOnRunTic", ModuleOnRunTic, 1, qtrue, qfalse },
	{ "ModuleOnSaveGame", ModuleOnSaveGame, 0, qfalse, qfalse },
	{ "ModuleOnLoadGame", ModuleOnLoadGame, 0, qfalse, qfalse },
	{ "ModuleOnJoystickEvent", ModuleOnJoystickEvent, 6, qfalse, qfalse },
};

CModuleHandle::CModuleHandle( const char *pName, const char *pDescription, const nlohmann::json& sourceFiles, int32_t moduleVersionMajor,
	int32_t moduleVersionUpdate, int32_t moduleVersionPatch, const nlohmann::json& includePaths, bool bIsDynamicModule
)
	: m_szName{ pName }, m_szDescription{ pDescription }, m_nStateStack{ 0 }, m_nVersionMajor{ moduleVersionMajor },
	m_nVersionUpdate{ moduleVersionUpdate }, m_nVersionPatch{ moduleVersionPatch }
{
	int error;
	char name[MAX_NPATH];

	PROFILE_BLOCK_BEGIN( "Compile Module" );

	m_bLoaded = qfalse;

	if ( !sourceFiles.size() ) {
		Con_Printf( COLOR_YELLOW "WARNING: no source files found for '%s', not compiling\n", pName );
		return;
	}

	if ( bIsDynamicModule ) {
		// don't compile it if we're possibly not even going to load it

		m_SourceFiles = eastl::move( sourceFiles );

		return;
	}

	g_pModuleLib->SetHandle( this );
	m_IncludePaths = eastl::move( includePaths );
	m_SourceFiles = eastl::move( sourceFiles );

	N_strncpyz( name, m_szName.c_str(), sizeof( name ) - 1 );
	g_pModuleLib->GetScriptBuilder()->DefineWord( va( "MODULE_%s", N_strupr( name ) ) );

//    m_pScriptModule->SetUserData( this );

	m_nLastCallId = NumFuncs;
	m_bLoaded = qtrue;
}

CModuleHandle::~CModuleHandle() {
	ClearMemory();
}

void CModuleHandle::Compile( void )
{
	Build( m_SourceFiles );
}

void CModuleHandle::LoadFunction( const string_t& moduleName, const string_t& funcName, asIScriptFunction **pFunction )
{
	const char *path;
	CModuleInfo *pInfo;
	asUINT i;

	path = FS_BuildOSPath( FS_GetHomePath(), NULL, va( "modules/%s", moduleName.c_str() ) );

	*pFunction = g_pModuleLib->GetScriptModule()->GetFunctionByName( funcName.c_str() );

	if ( !*pFunction ) {
		Con_Printf( COLOR_RED "Error loading dynamic function pointer \"%s\".\n", funcName.c_str() );
	}
}

void CModuleHandle::Build( const nlohmann::json& sourceFiles ) {
	int error;

	for ( const auto& it : sourceFiles ) {
		const string_t& file = it;
		if ( !LoadSourceFile( file ) ) {
			// unless the game cannot run without it, we'll just scream at the console
			if ( IsRequiredModule( m_szName.c_str() ) ) {
				N_Error( ERR_DROP, "CModuleHandle::Build: failed to compile source file '%s'", file.c_str() );
			} else {
				Con_Printf( COLOR_RED "CModuleHandle::Build: failed to compile source file '%s'\n", file.c_str() );
			}
		}
	}
}

const char *CModuleHandle::GetModulePath( void ) const {
	return va( "modules/%s/", m_szName.c_str() );
}

void LogExceptionInfo( asIScriptContext *pContext, void *userData )
{
	const asIScriptFunction *pFunc;
	const CModuleHandle *pHandle;
	char msg[4096];
	const char *err;

	pFunc = pContext->GetExceptionFunction();
	pHandle = (const CModuleHandle *)userData;
	err = Cvar_VariableString( "com_errorMessage" );

	Con_Printf( COLOR_RED "ERROR: exception thrown by module \"%s\": %s\n", pHandle->GetName().c_str(), err );
	Con_Printf( COLOR_RED "Printing stacktrace...\n" );
	Cbuf_ExecuteText( EXEC_NOW, va( "ml_debug.set_active \"%s\"", pHandle->GetName().c_str() ) );
	g_pDebugger->PrintCallstack( pContext );

	if ( *err ) {
		Com_snprintf( msg, sizeof( msg ) - 1,
			"exception was thrown in module ->\n"
			" Module ID: %s\n"
			" Section Name: %s\n"
			" Function: %s\n"
			" Line: %i\n"
			" Error Type: %s\n"
			" Error Message: %s\n"
		, pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
		pContext->GetExceptionString(), err );
	} else {
		Com_snprintf( msg, sizeof( msg ) - 1,
			"exception was thrown in module ->\n"
			" Module ID: %s\n"
			" Section Name: %s\n"
			" Function: %s\n"
			" Line: %i\n"
			" Error Message: %s\n"
		, pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
		pContext->GetExceptionString() );
	}

	N_Error( ERR_DROP, "%s", msg );
}

void CModuleHandle::PrepareContext( asIScriptFunction *pFunction )
{
}

int CModuleHandle::CallFunc( EModuleFuncId nCallId, uint32_t nArgs, int *pArgList )
{
	uint32_t i;
	int retn;
	asIScriptContext *pContext;

	pContext = g_pModuleLib->GetScriptContext();

	PROFILE_BLOCK_BEGIN( va( "module '%s'", m_szName.c_str() ) );

	if ( !m_pFuncTable[ nCallId ] ) {
		return 1; // just wasn't registered, any required func procs will have thrown an error in init stage
	}

	// always ensure the handle is set for any calls into the engine that'll modify the module itself
	g_pModuleLib->SetHandle( this );

	CheckASCall( pContext->SetExceptionCallback( asFUNCTION( LogExceptionInfo ), this, asCALL_CDECL ) );

	// prevent a nested call infinite recursion
	if ( m_nLastCallId == nCallId ) {
		N_Error( ERR_DROP, "CModuleHandle::CallFunc: recursive nested module call" );
	}
	m_nLastCallId = nCallId;

	if ( pContext->GetState() == asEXECUTION_ACTIVE ) {
		// we're running something right now, so push a new state and
		// call into that instead
		CheckASCall( pContext->PushState() );
		CheckASCall( pContext->Prepare( m_pFuncTable[ nCallId ] ) );

		if ( ml_debugMode->i && g_pDebugger->m_pModule && g_pDebugger->m_pModule->m_pHandle == this ) {
			CheckASCall( pContext->SetLineCallback( asMETHOD( CDebugger, LineCallback ), g_pDebugger, asCALL_THISCALL ) );
		}
	
		g_pModuleLib->SetHandle( this );
	
		for ( i = 0; i < nArgs; i++ ) {
			CheckASCall( pContext->SetArgDWord( i, pArgList[i] ) );
		}
		
		try {
			retn = pContext->Execute();
		} catch ( const ModuleException& e ) {
			LogExceptionInfo( pContext, this );
		} catch ( const nlohmann::json::exception& e ) {
			const asIScriptFunction *pFunc;
			pFunc = pContext->GetExceptionFunction();
		
			N_Error( ERR_DROP,
				"nlohmann::json::exception was thrown in module ->\n"
				" Module ID: %s\n"
				" Section Name: %s\n"
				" Function: %s\n"
				" Line: %i\n"
				" Error Message: %s\n"
				" Id: %i\n"
			, pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
			e.what(), e.id );
		}
	
		switch ( retn ) {
		case asEXECUTION_ABORTED:
		case asEXECUTION_ERROR:
		case asEXECUTION_EXCEPTION:
			// something happened in there, dunno what
			LogExceptionInfo( pContext, this );
			break;
		case asEXECUTION_SUSPENDED:
		case asEXECUTION_FINISHED:
		default:
			// exited successfully
			break;
		};
	
		retn = (int)pContext->GetReturnWord();
	
		PROFILE_BLOCK_END;

		CheckASCall( pContext->PopState() );

		return retn;
	}
	CheckASCall( pContext->Prepare( m_pFuncTable[ nCallId ] ) );

	g_pModuleLib->GetScriptEngine()->GarbageCollect( asGC_DETECT_GARBAGE, 1 );

	if ( ml_debugMode->i && g_pDebugger->m_pModule && g_pDebugger->m_pModule->m_pHandle == this ) {
		CheckASCall( pContext->SetLineCallback( asMETHOD( CDebugger, LineCallback ), g_pDebugger, asCALL_THISCALL ) );
	}

	g_pModuleLib->SetHandle( this );

	for ( i = 0; i < nArgs; i++ ) {
		CheckASCall( pContext->SetArgDWord( i, pArgList[i] ) );
	}
	
	retn = 0;
	try {
		retn = pContext->Execute();
	} catch ( const ModuleException& e ) {
		LogExceptionInfo( pContext, this );
	} catch ( const nlohmann::json::exception& e ) {
		const asIScriptFunction *pFunc;
		pFunc = pContext->GetExceptionFunction();
	
		N_Error( ERR_DROP,
			"nlohmann::json::exception was thrown in module ->\n"
			" Module ID: %s\n"
			" Section Name: %s\n"
			" Function: %s\n"
			" Line: %i\n"
			" Error Message: %s\n"
			" Id: %i\n"
		,  pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
		e.what(), e.id );
	}

	switch ( retn ) {
	case asEXECUTION_ABORTED:
	case asEXECUTION_ERROR:
	case asEXECUTION_EXCEPTION:
		// something happened in there, dunno what
		LogExceptionInfo( pContext, this );
		break;
	case asEXECUTION_SUSPENDED:
	case asEXECUTION_FINISHED:
	default:
		// exited successfully
		break;
	};

	retn = (int)pContext->GetReturnWord();

	PROFILE_BLOCK_END;

	m_nLastCallId = NumFuncs;

	return retn;
}

void CModuleHandle::RegisterGameObject( void )
{
}

bool CModuleHandle::InitCalls( void )
{
	uint32_t i;
	char szFuncName[1024];

	Con_Printf( "Initializing function procs...\n" );

	memset( m_pFuncTable, 0, sizeof( m_pFuncTable ) );

	for ( i = 0; i < NumFuncs; i++ ) {
		Com_snprintf( szFuncName, sizeof( szFuncName ) - 1, "%s::%s", m_szName.c_str(), funcDefs[i].name );

		Con_DPrintf( "Checking if module has function '%s'...\n", szFuncName );
		m_pFuncTable[i] = g_pModuleLib->GetScriptModule()->GetFunctionByName( szFuncName );
		if ( m_pFuncTable[i] ) {
			Con_Printf( COLOR_GREEN "Module \"%s\" registered with proc '%s'.\n", m_szName.c_str(), szFuncName );
		} else {
			if ( funcDefs[i].required ) {
				Con_Printf( COLOR_RED "Module \"%s\" not registered with required proc '%s'.\n", m_szName.c_str(), szFuncName );
				return false;
			}
			Con_Printf( COLOR_MAGENTA "Module \"%s\" not registered with proc '%s'.\n", m_szName.c_str(), szFuncName );
			continue;
		}

		if ( funcDefs[i].mainOnly && m_pFuncTable[i] && !IsRequiredModule( m_szName.c_str() ) ) {
			Con_Printf( COLOR_RED "Module \"%s\" has an exclusive proc, ignoring.\n", m_szName.c_str() );
			m_pFuncTable[i] = NULL;
			continue;
		}
		if ( m_pFuncTable[i]->GetReturnTypeId() != asTYPEID_INT32 ) {
			Con_Printf( COLOR_RED "Module \"%s\" has proc '%s' but doesn't return an int.\n", m_szName.c_str(), szFuncName );
			return false;
		}
		if ( m_pFuncTable[i]->GetParamCount() != funcDefs[i].expectedArgs ) {
			Con_Printf( COLOR_RED "Module \"%s\" has proc '%s' but not the correct args (%u).\n", m_szName.c_str(), szFuncName,
				funcDefs[i].expectedArgs );
			return false;
		}
	}
	return true;
}

void CModuleHandle::AddDefines( Preprocessor& preprocessor ) const
{
//    preprocessor.ReserveDefines( 1024 );

	preprocessor.define( va( "MODULE_NAME \"%s\"", m_szName.c_str() ) );
	preprocessor.define( va( "MODULE_VERSION_MAJOR %i", m_nVersionMajor ) );
	preprocessor.define( va( "MODULE_VERSION_UPDATE %i", m_nVersionUpdate ) );
	preprocessor.define( va( "MODULE_VERSION_PATCH %i", m_nVersionPatch ) );

	preprocessor.define( va( "GAME_NAME \"%s\"", GLN_VERSION ) );
	preprocessor.define( va( "NOMAD_VERSION_STRING \"%u\"", NOMAD_VERSION_FULL ) );
	preprocessor.define( va( "NOMAD_VERSION %u", NOMAD_VERSION ) );
	preprocessor.define( va( "NOMAD_VERSION_UPDATE %u", NOMAD_VERSION_UPDATE ) );
	preprocessor.define( va( "NOMAD_VERSION_PATCH %u", NOMAD_VERSION_PATCH ) );

	preprocessor.define( va( "ENTITYNUM_INVALID %i", ENTITYNUM_INVALID ) );
	preprocessor.define( va( "ENTITYNUM_WALL %i", ENTITYNUM_WALL ) );

	preprocessor.define( va( "RSF_NOWORLDMODEL %u", RSF_NOWORLDMODEL ) );
	preprocessor.define( va( "RSF_ORTHO_BITS %u", RSF_ORTHO_BITS ) );
	preprocessor.define( va( "RSF_ORTHO_TYPE_CORDESIAN %u", RSF_ORTHO_TYPE_CORDESIAN ) );
	preprocessor.define( va( "RSF_ORTHO_TYPE_SCREENSPACE %u", RSF_ORTHO_TYPE_SCREENSPACE ) );
	preprocessor.define( va( "RSF_ORTHO_TYPE_WORLD %u", RSF_ORTHO_TYPE_WORLD ) );

	preprocessor.define( va( "FS_INVALID_HANDLE %i", FS_INVALID_HANDLE ) );
	preprocessor.define( va( "FS_OPEN_APPEND %i", FS_OPEN_APPEND ) );
	preprocessor.define( va( "FS_OPEN_READ %i", FS_OPEN_READ ) );
	preprocessor.define( va( "FS_OPEN_WRITE %i", FS_OPEN_WRITE ) );
	preprocessor.define( va( "FS_OPEN_RW %i", FS_OPEN_RW ) );

	preprocessor.define( va( "S_COLOR_RED %i", S_COLOR_RED ) );
	preprocessor.define( va( "S_COLOR_GREEN %i", S_COLOR_GREEN ) );
	preprocessor.define( va( "S_COLOR_YELLOW %i", S_COLOR_YELLOW ) );
	preprocessor.define( va( "S_COLOR_BLUE %i", S_COLOR_BLUE ) );
	preprocessor.define( va( "S_COLOR_CYAN %i", S_COLOR_CYAN ) );
	preprocessor.define( va( "S_COLOR_MAGENTA %i", S_COLOR_MAGENTA ) );
	preprocessor.define( va( "S_COLOR_WHITE %i", S_COLOR_WHITE ) );
	preprocessor.define( va( "S_COLOR_RESET %i", S_COLOR_RESET ) );

	preprocessor.define( va( "COLOR_BLACK \"%s\"", COLOR_BLACK ) );
	preprocessor.define( va( "COLOR_RED \"%s\"", COLOR_RED ) );
	preprocessor.define( va( "COLOR_GREEN \"%s\"", COLOR_GREEN ) );
	preprocessor.define( va( "COLOR_YELLOW \"%s\"", COLOR_YELLOW ) );
	preprocessor.define( va( "COLOR_BLUE \"%s\"", COLOR_BLUE ) );
	preprocessor.define( va( "COLOR_CYAN \"%s\"", COLOR_CYAN ) );
	preprocessor.define( va( "COLOR_MAGENTA \"%s\"", COLOR_MAGENTA ) );
	preprocessor.define( va( "COLOR_WHITE \"%s\"", COLOR_WHITE ) );
	preprocessor.define( va( "COLOR_RESET \"%s\"", COLOR_RESET ) );

	preprocessor.define( va( "CVAR_CHEAT %i", CVAR_CHEAT ) );
	preprocessor.define( va( "CVAR_ROM %i", CVAR_ROM ) );
	preprocessor.define( va( "CVAR_INIT %i", CVAR_INIT ) );
	preprocessor.define( va( "CVAR_LATCH %i", CVAR_LATCH ) );
	preprocessor.define( va( "CVAR_NODEFAULT %i", CVAR_NODEFAULT ) );
	preprocessor.define( va( "CVAR_NORESTART %i", CVAR_NORESTART ) );
	preprocessor.define( va( "CVAR_NOTABCOMPLETE %i", CVAR_NOTABCOMPLETE ) );
	preprocessor.define( va( "CVAR_TEMP %i", CVAR_TEMP ) );
	preprocessor.define( va( "CVAR_SAVE %i", CVAR_SAVE ) );

	preprocessor.define( va( "MAXPRINTMSG %i", MAXPRINTMSG ) );

	preprocessor.define( va( "SURFACEPARM_WOOD %i", SURFACEPARM_WOOD ) );
	preprocessor.define( va( "SURFACEPARM_METAL %i", SURFACEPARM_METAL ) );
	preprocessor.define( va( "SURFACEPARM_FLESH %i", SURFACEPARM_FLESH ) );
	preprocessor.define( va( "SURFACEPARM_WATER %i", SURFACEPARM_WATER ) );
	preprocessor.define( va( "SURFACEPARM_LAVA %i", SURFACEPARM_LAVA ) );
	preprocessor.define( va( "SURFACEPARM_NOSTEPS %i", SURFACEPARM_NOSTEPS ) );
	preprocessor.define( va( "SURFACEPARM_NODAMAGE %i", SURFACEPARM_NODAMAGE ) );
	preprocessor.define( va( "SURFACEPARM_NODLIGHT %i", SURFACEPARM_NODLIGHT ) );
	preprocessor.define( va( "SURFACEPARM_NOMARKS %i", SURFACEPARM_NOMARKS ) );
	preprocessor.define( va( "SURFACEPARM_NOMISSILE %i", SURFACEPARM_NOMISSILE ) );
	preprocessor.define( va( "SURFACEPARM_SLICK %i", SURFACEPARM_SLICK ) );
	preprocessor.define( va( "SURFACEPARM_LIGHTFILTER %i", SURFACEPARM_LIGHTFILTER ) );
	preprocessor.define( va( "SURFACEPARM_ALPHASHADOW %i", SURFACEPARM_ALPHASHADOW ) );
	preprocessor.define( va( "SURFACEPARM_LADDER %i", SURFACEPARM_LADDER ) );
	preprocessor.define( va( "SURFACEPARM_NODRAW %i", SURFACEPARM_NODRAW ) );
	preprocessor.define( va( "SURFACEPARM_POINTLIGHT %i", SURFACEPARM_POINTLIGHT ) );
	preprocessor.define( va( "SURFACEPARM_NOLIGHTMAP %i", SURFACEPARM_NOLIGHTMAP ) );
	preprocessor.define( va( "SURFACEPARM_DUST %i", SURFACEPARM_DUST ) );
	preprocessor.define( va( "SURFACEPARM_NONSOLID %i", SURFACEPARM_NONSOLID ) );
}

bool CModuleHandle::LoadSourceFile( const string_t& filename )
{
	union {
		void *v;
		char *b;
	} f;
	int retn, errCount;
	const char *path;
	uint64_t length;
	UtlString data;

	path = va( "modules/%s/%s", m_szName.c_str(), filename.c_str() );
	length = FS_LoadFile( path, &f.v );
	if ( !f.v ) {
		Con_Printf( COLOR_RED "ERROR: Couldn't load script file '%s'.\n", path );
		return false;
	}

	errCount = 0;

	if ( errCount > 0 ) {
		retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
		if ( retn < 0 ) {
			Con_Printf( COLOR_RED "ERROR: failed to compile source file '%s' -- %s, %i errors\n", filename.c_str(),
				AS_PrintErrorString( retn ), errCount );
			return false;
		}
	} else {
		retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
		if ( retn < 0 ) {
			Con_Printf( COLOR_RED "ERROR: failed to compile source file '%s' -- %s\n", filename.c_str(), AS_PrintErrorString( retn ) );
			return false;
		}
	}
	FS_FreeFile( f.v );

	return true;
}

/*
* CModuleHandle::ClearMemory: called whenever exiting
*/
void CModuleHandle::ClearMemory( void )
{
	uint64_t i;

	if ( g_pModuleLib->GetScriptContext()->GetState() == asEXECUTION_ACTIVE ) {
		g_pModuleLib->GetScriptContext()->Abort();
	}

	Con_Printf( "CModuleHandle::ClearMemory: clearing memory of '%s'...\n", m_szName.c_str() );

	for ( i = 0; i < NumFuncs; i++ ) {
		if ( m_pFuncTable[i] ) {
			m_pFuncTable[i]->Release();
		}
	}

	CheckASCall( g_pModuleLib->GetScriptContext()->Unprepare() );
}

const string_t& CModuleHandle::GetName( void ) const {
	return m_szName;
}

CModuleCacheHandle::CModuleCacheHandle( const char *path, fileMode_t mode ) {
	uint64_t nLength;

	nLength = FS_FOpenFileWithMode( path, &m_hFile, mode );
	if ( m_hFile == FS_INVALID_HANDLE ) {
		N_Error( ERR_DROP, "CModuleCacheHande::CModuleCacheHandle: failed to open '%s'", path );
	}
}

CModuleCacheHandle::~CModuleCacheHandle() {
	FS_FClose( m_hFile );
}

int CModuleCacheHandle::Read( void *pBuffer, asUINT nBytes ) {
	if ( !nBytes ) {
		Assert( nBytes );
		return 0;
	}
	return FS_Read( pBuffer, nBytes, m_hFile );
}

int CModuleCacheHandle::Write( const void *pBuffer, asUINT nBytes ) {
	if ( !nBytes ) {
		Assert( nBytes );
		return 0;
	}
	return FS_Write( pBuffer, nBytes, m_hFile );
}
