// module_main.cpp -- initializes the module library

#include "module_public.h"
#include "angelscript/angelscript.h"
#include "module_handle.h"
#include "../game/g_game.h"
#include <glm/glm.hpp>
#include <filesystem>
#include "scriptlib/scriptjson.h"
#include "module_renderlib.h"
#include "module_funcdefs.hpp"
#include "module_stringfactory.hpp"
#include "module_debugger.h"
#include "scriptlib/scriptarray.h"
#include "scriptlib/scriptdictionary.h"
#include "scriptlib/scriptany.h"

moduleImport_t moduleImport;

static CModuleLib *s_pModuleInstance;

cvar_t *ml_angelScript_DebugPrint;
cvar_t *ml_debugMode;
cvar_t *ml_alwaysCompile;
cvar_t *ml_allowJIT;
cvar_t *ml_garbageCollectionIterations;

static void ML_CleanCache_f( void ) {
	const char *path;
	uint64_t i;
	const CModuleInfo *it;

	Con_Printf( "Clearing cached bytecode script files...\n" );
	for ( i = 0; i < g_pModuleLib->GetModCount(); i++ ) {
		it = &g_pModuleLib->GetLoadList()[ i ];

		path = va( CACHE_DIR "/%s_code.dat", it->m_pHandle->GetName().c_str() );
		FS_Remove( path );
		FS_HomeRemove( path );

		path = va( CACHE_DIR "/%s_metadata.bin", it->m_pHandle->GetName().c_str() );
		FS_Remove( path );
		FS_HomeRemove( path );
	}
}

static void ML_GarbageCollectionStats_f( void ) {
	asUINT currentSize, totalDestroyed, totalDetected, newObjects, totalNewDestroyed;
	g_pModuleLib->GetScriptEngine()->GetGCStatistics( &currentSize, &totalDestroyed, &totalDetected, &newObjects, &totalNewDestroyed );

	Con_Printf( "--------------------\n" );
	Con_Printf( "[Garbage Collection Stats]\n" );
	Con_Printf( "Current Size: %u\n", currentSize );
	Con_Printf( "Total Destroyed: %u\n", totalDestroyed );
	Con_Printf( "Total Detected: %u\n", totalDetected );
	Con_Printf( "New Objects: %u\n", newObjects );
	Con_Printf( "Total New Destroyed: %u\n", totalNewDestroyed );
	Con_Printf( "--------------------\n" );
}

const char *AS_PrintErrorString( int code )
{
	switch ( code ) {
	case asERROR:
		return "error occurred";
	case asNOT_SUPPORTED:
		return "not supported";
	case asINVALID_ARG:
		return "invalid argument";
	case asINVALID_CONFIGURATION:
		return "invalid configuration";
	case asINVALID_DECLARATION:
		return "invalid declaration";
	case asINVALID_INTERFACE:
		return "invalid interface";
	case asINVALID_NAME:
		return "invalid name";
	case asINVALID_OBJECT:
		return "invalid object";
	case asINVALID_TYPE:
		return "invalid type";
	case asNO_FUNCTION:
		return "function wasn't found";
	case asNO_GLOBAL_VAR:
		return "no global variable was found";
	case asNAME_TAKEN:
		return "name already taken";
	case asMULTIPLE_FUNCTIONS:
		return "multiple matching functions";
	case asCANT_BIND_ALL_FUNCTIONS:
		return "could not bind all imported module functions";
	case asCONTEXT_NOT_FINISHED:
		return "context not finished";
	case asCONTEXT_NOT_PREPARED:
		return "context not prepared";
	case asOUT_OF_MEMORY:
		return "out of memory... SOMEHOW?";
	case asILLEGAL_BEHAVIOUR_FOR_TYPE:
		return "illegal behaviour for type";
	case asWRONG_CALLING_CONV:
		return "specified calling convention doesn't match the function/method pointer";
	case asALREADY_REGISTERED:
		return "already registered";
	case asINIT_GLOBAL_VARS_FAILED:
		return "initialization of global vars failed";
	case asNO_MODULE:
		return "no module was found";
	case asWRONG_CONFIG_GROUP:
		return "already registered in another config group";
	case asLOWER_ARRAY_DIMENSION_NOT_REGISTERED:
		return "array templated subtype not registered yet";
	case asMODULE_IS_IN_USE:
		return "module is referred to by active object or from the engine";
	default:
		break;
	};
	return va( "<unknown error> (%i)", code );
}

namespace EA::StdC {

extern "C" EASTL_EASTDC_API int Vsnprintf(char*  EA_RESTRICT pDestination, size_t n, const char*  EA_RESTRICT pFormat, va_list arguments) {
	return N_vsnprintf( pDestination, n, pFormat, arguments );
}

EASTL_EASTDC_API int Vsnprintf(char16_t* EA_RESTRICT pDestination, size_t n, const char16_t* EA_RESTRICT pFormat, va_list arguments) {
	return 0;
}

// who the fuck even uses a char32?
EASTL_EASTDC_API int Vsnprintf(char32_t* EA_RESTRICT pDestination, size_t n, const char32_t* EA_RESTRICT pFormat, va_list arguments) {
	return 0;
}

#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
	EASTL_EASTDC_API int Vsnprintf(wchar_t* EA_RESTRICT pDestination, size_t n, const wchar_t* EA_RESTRICT pFormat, va_list arguments) {
		return 0; // TODO
	}
#endif
};

void CModuleLib::LoadModule( const char *pModule )
{
	PROFILE_FUNCTION();

	nlohmann::json parse;
	nlohmann::json includePaths;
	string_t description;
	CModuleHandle *pHandle;
	union {
		void *v;
		char *b;
	} f;

	try {
		uint64_t length = FS_LoadFile( va( "modules/%s/module.json", pModule ), &f.v );
		if ( !length || !f.v ) {
			Con_Printf( COLOR_RED "ERROR: failed to load module configuration for \"%s\"!\n", pModule );
			return;
		} else {
			parse = nlohmann::json::parse( f.b, f.b + length );
		}
		FS_FreeFile( f.v );
		
	} catch ( const nlohmann::json::exception& e ) {
		Con_Printf( COLOR_RED "ERROR: failed to load module configuration for \"%s\"!\n\tid: %i\n\tmessage: %s\n", pModule, e.id, e.what() );
		if ( f.v ) {
			FS_FreeFile( f.v );
		}
		return;
	}

	//
	// validate its an actual config
	//
	if ( !parse.contains( "SubModules" ) ) {
		Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no array 'SubModules'\n", pModule );
		return;
	}
	if ( !parse.contains( "DependedModules" ) ) {
		Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no array 'DependedModules'\n", pModule );
		return;
	}
	if ( parse.contains( "Version" ) ) {
		if ( !parse.at( "Version" ).contains( "GameVersionMajor" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'GameVersionMajor'\n", pModule );
			return;
		}
		if ( !parse.at( "Version" ).contains( "GameVersionUpdate" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'GameVersionUpdate'\n", pModule );
			return;
		}
		if ( !parse.at( "Version" ).contains( "GameVersionPatch" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'GameVersionPatch'\n", pModule );
			return;
		}
		if ( !parse.at( "Version" ).contains( "VersionMajor" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'VersionMajor'\n", pModule );
			return;
		}
		if ( !parse.at( "Version" ).contains( "VersionUpdate" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'VersionUpdate'\n", pModule );
			return;
		}
		if ( !parse.at( "Version" ).contains( "VersionPatch" ) ) {
			Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'VersionPatch'\n", pModule );
			return;
		}
	} else {
		Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no object 'version'\n", pModule );
		return;
	}

	if ( parse.contains( "IncludePaths" ) ) {
		includePaths = eastl::move( parse.at( "IncludePaths" ) );

		//
		// check if it already has the default path
		//
		bool found = false;
		const char *path = va( "%s/", pModule );
		for ( const auto& it : includePaths ) {
			if ( !N_strcmp( it.get<string_t>().c_str(), path ) ) {
				found = true;
				break;
			}
		}
		if ( !found ) {
			includePaths.emplace_back( path );
		}
	} else {
		includePaths.emplace_back( va( "%s/", pModule ) );
	}
	description = parse.contains( "Description" ) ? parse[ "Description" ].get<string_t>() : "";
	const int32_t versionMajor = parse[ "Version" ][ "VersionMajor" ];
	const int32_t versionUpdate = parse[ "Version" ][ "VersionUpdate" ];
	const int32_t versionPatch = parse[ "Version" ][ "VersionPatch" ];

	pHandle = new ( Hunk_Alloc( sizeof( *pHandle ), h_high ) ) CModuleHandle( pModule, description.c_str(), parse.at( "SubModules" ),
		versionMajor, versionUpdate, versionPatch, includePaths, parse.at( "IsDynamic" ) );
	new ( &m_pLoadList[ m_nModuleCount ] ) CModuleInfo( parse, pHandle );
	m_nModuleCount++;
}

void CModuleLib::RunModules( EModuleFuncId nCallId, uint32_t nArgs, ... )
{
	va_list argptr;
	uint64_t j;
	uint32_t i;
	int *args;
	CTimer time;

	if ( nCallId >= NumFuncs ) {
		N_Error( ERR_FATAL, "CModuleLib::RunModules: invalid call id" );
	}
	
	args = (int *)alloca( sizeof( *args ) * nArgs );
	va_start( argptr, nArgs );
	for ( i = 0; i < nArgs; i++ ) {
		args[i] = va_arg( argptr, int );
	}
	va_end( argptr );

	time.Start();
	if ( nCallId == ModuleOnLevelEnd || nCallId == ModuleOnLoadGame || nCallId == ModuleShutdown ) {
		g_pModuleLib->GetScriptEngine()->GarbageCollect( asGC_DETECT_GARBAGE | asGC_DESTROY_GARBAGE
			| asGC_FULL_CYCLE, (uint32_t)ml_garbageCollectionIterations->i );
	}
	time.Stop();

	for ( j = 0; j < m_nModuleCount; j++ ) {
		if ( sgvm == &m_pLoadList[j] ) {
			continue; // avoid running it twice
		}
		if ( m_pLoadList[j].m_pHandle->CallFunc( nCallId, nArgs, args ) == -1 ) {
			Con_Printf( COLOR_YELLOW "WARNING: module \"%s\" returned error code on call to proc \"%s\".\n",
				m_pLoadList[i].m_szName, funcDefs[ nCallId ].name );
		}
	}
}

int CModuleLib::ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... )
{
	va_list argptr;
	uint32_t i;
	int *args;
	const char *name;
	CTimer time;

	if ( !pModule ) {
		N_Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid module" );
	}
	if ( nCallId >= NumFuncs ) {
		N_Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid call id" );
	}
	
	args = (int *)alloca( sizeof( *args ) * nArgs );
	va_start( argptr, nArgs );
	for ( i = 0; i < nArgs; i++ ) {
		args[i] = va_arg( argptr, int );
	}
	va_end( argptr );

	name = funcDefs[ nCallId ].name;

	time.Start();
	if ( nCallId == ModuleOnLevelEnd || nCallId == ModuleOnLoadGame || nCallId == ModuleShutdown ) {
		g_pModuleLib->GetScriptEngine()->GarbageCollect( asGC_DETECT_GARBAGE | asGC_DESTROY_GARBAGE
			| asGC_FULL_CYCLE, (uint32_t)ml_garbageCollectionIterations->i );
	}
	time.Stop();

	return pModule->m_pHandle->CallFunc( nCallId, nArgs, args );
}

void Module_ASMessage_f( const asSMessageInfo *pMsg, void *param )
{
	bool error;

	error = false;

	switch ( pMsg->type ) {
	case asMSGTYPE_ERROR:
		Con_Printf( COLOR_RED "ERROR: [AngelScript](%s:%i:%i) %s\n",
			pMsg->section, pMsg->row, pMsg->col, pMsg->message );
		error = true;
		break;
	case asMSGTYPE_WARNING:
		Con_Printf( COLOR_YELLOW "WARNING: [AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
		break;
	case asMSGTYPE_INFORMATION: {
		if ( !ml_angelScript_DebugPrint->i ) {
			return;
		}
		Con_Printf( "[AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
		break; }
	};

	if ( error ) {
		//Sys_MessageBox( "*** ERROR DURING MODULE COMPILATION ***",
		//    va( "An error occurred during the compilation of a module section\n"
		//        "%s:%i:%i %s"
		//    , pMsg->section, pMsg->row, pMsg->col, pMsg->message ),
		//    false );
	}
}

void *AS_Alloc( size_t nSize ) {
//    return Hunk_Alloc( nSize, h_high );
	return Mem_Alloc( nSize );
}
void AS_Free( void *ptr ) {
//    Z_Free( ptr );
	Mem_Free( ptr );
}

/*
* AS_Printf: a debugging tool used for whenever the angelscript compiler decides
* to break on me
*/
void GDR_ATTRIBUTE((format(printf, 1, 2))) AS_Printf( const char *fmt, ... )
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	if ( !ml_angelScript_DebugPrint->i ) {
		return;
	}

	va_start( argptr, fmt );
	N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
	va_end( argptr );

	Con_Printf( COLOR_GREEN "[AngelScript DEBUG]: %s", msg );
}

int Module_IncludeCallback_f( const char *pInclude, const char *pFrom, CScriptBuilder *pBuilder, void *unused )
{
	union {
		void *v;
		char *b;
	} f;
	uint64_t length;
	const char *path;
	const nlohmann::json& includePaths = g_pModuleLib->GetCurrentHandle()->GetIncludePaths();

	for ( const auto& it : includePaths ) {
		path = va( "%s%s", it.get<string_t>().c_str(), pInclude );
		length = FS_LoadFile( path, &f.v );
		if ( !f.v ) {
			continue;
		}

		pBuilder->AddSectionFromMemory( COM_SkipPath( const_cast<char *>( pInclude ) ), f.b, length );
		FS_FreeFile( f.v );

		g_pModuleLib->GetScriptEngine()->WriteMessage( g_pModuleLib->GetCurrentHandle()->GetModulePath(),
			0, 0, asMSGTYPE_INFORMATION, va( "Added include file \"%s\" to \"%s\"\n", path, pFrom ) );
		return 1;
	}

	(void)unused; // shut up compiler
	g_pModuleLib->GetScriptEngine()->WriteMessage(
		g_pModuleLib->GetCurrentHandle()->GetModulePath(),
		0, 0, asMSGTYPE_WARNING, va( "failed to load include preprocessor file \"%s\" from \"%s\"", pInclude, pFrom ) );
	return -1;
}

bool CModuleLib::AddDefaultProcs( void ) const {
	if ( m_bRegistered ) {
		return true;
	}

	RegisterScriptArray( m_pEngine );
	RegisterStdString( g_pModuleLib->GetScriptEngine() );
	RegisterScriptHandle( m_pEngine );
	RegisterScriptAny( m_pEngine );
	RegisterScriptParser( m_pEngine );

	RegisterScriptDictionary( m_pEngine );
	RegisterScriptMath( m_pEngine );
	RegisterScriptJson( m_pEngine );

	ModuleLib_Register_Engine();
	const_cast<CModuleLib *>( this )->m_bRegistered = qtrue;

	return true;
}

#define AS_CACHE_CODE_IDENT (('C'<<24)+('B'<<16)+('S'<<8)+'A')

static fileTime_t ModuleLib_GetDirectoryChecksum( const char *dir )
{
	char **fileList, **dirList;
	uint64_t numFiles, numDirs;
	uint64_t i;
	fileStats_t stats;
	fileTime_t total;
	char szDirectory[ MAX_OSPATH ];
	char szPath[ MAX_OSPATH*2+1 ];

	total = 0;

	memset( szDirectory, 0, sizeof( szDirectory ) );

	if ( !N_stristr( dir, "gamedata" ) ) {
		snprintf( szDirectory, sizeof( szDirectory ) - 1, "gamedata/%s", dir );
	} else {
		N_strncpyz( szDirectory, dir, sizeof( szDirectory ) );
	}

	dirList = FS_ListFiles( szDirectory, "/", &numDirs );
	for ( i = 0; i < numDirs; i++ ) {
		if ( *dirList[i] == '.' || ( *dirList[i] == '.' && *( dirList[i]+1 ) == '.' ) ) {
			continue;
		}
		if ( dirList[i][ strlen( dirList[i] ) - 1 ] != '/' ) {
			snprintf( szPath, sizeof( szPath ) - 1, "%s%s/", szDirectory, dirList[i] );
		} else {
			snprintf( szPath, sizeof( szPath ) - 1, "%s%s", szDirectory, dirList[i] );
		}
		total += ModuleLib_GetDirectoryChecksum( szPath );
		Con_Printf( "..fetched checksum of directory '%s'\n", szPath );
	}
	FS_FreeFileList( dirList );

	fileList = FS_ListFiles( szDirectory, ".as", &numFiles );
	for ( i = 0; i < numFiles; i++ ) {
		if ( szDirectory[ strlen( szDirectory ) - 1 ] != '/' ) {
			snprintf( szPath, sizeof( szPath ) - 1, "%s/%s", szDirectory, fileList[i] );
		} else {
			snprintf( szPath, sizeof( szPath ) - 1, "%s%s", szDirectory, fileList[i] );
		}
		if ( !Sys_GetFileStats( &stats, szPath ) ) {
			N_Error( ERR_DROP, "ModuleLib_GetDirectoryChecksum: couldn't get filestats for '%s'", szPath );
		}
		total += stats.mtime;
	}
	FS_FreeFileList( fileList );

	return total;
}

void CModuleLib::SaveByteCodeCache( void )
{
	int ret;
	asCodeCacheHeader_t *header;
	CModuleCacheHandle dataStream( va( CACHE_DIR "/ascodecache.dat" ), FS_OPEN_WRITE );
	byte *pByteCode;
	uint64_t nLength;
	fileHandle_t hFile;
	uint64_t i;
	char *ptr;

	const CModuleInfo *pModule = GetModule( "nomadmain" );

	header = (asCodeCacheHeader_t *)Hunk_AllocateTempMemory( sizeof( *header ) + ( MAX_NPATH * m_nModuleCount ) );
	header->hasDebugSymbols = ml_debugMode->i;
	header->gameVersion.m_nVersionMajor = _NOMAD_VERSION_MAJOR;
	header->gameVersion.m_nVersionUpdate = _NOMAD_VERSION_UPDATE;
	header->gameVersion.m_nVersionPatch = _NOMAD_VERSION_PATCH;
	header->ident = AS_CACHE_CODE_IDENT;
	header->moduleVersionMajor = pModule->m_nModVersionMajor;
	header->moduleVersionUpdate = pModule->m_nModVersionUpdate;
	header->moduleVersionPatch = pModule->m_nModVersionPatch;
	header->modCount = m_nModuleCount;
	header->checksum = 0;
	for ( i = 0; i < m_nModuleCount; i++ ) {
		header->checksum += ModuleLib_GetDirectoryChecksum( m_pModList[i].info->m_pHandle->GetModulePath() );
	}
	Con_Printf( "Total module checksum is %lu\n", header->checksum );
	
	ptr = header->modList;
	for ( i = 0; i < m_nModuleCount; i++ ) {
		memcpy( ptr, m_pModList[ i ].info->m_szId, MAX_NPATH );
		ptr += MAX_NPATH;
	}

	ret = m_pModule->SaveByteCode( &dataStream, header->hasDebugSymbols );
	if ( ret != asSUCCESS ) {
		Con_Printf( COLOR_RED "ERROR: failed to save module bytecode cache\n" );
	}
	FS_FClose( dataStream.m_hFile );

	Hunk_FreeTempMemory( header );

	FS_WriteFile( va( CACHE_DIR "/asmetadata.bin" ), header, sizeof( *header ) + ( MAX_NPATH * m_nModuleCount ) );
}

qboolean CModuleLib::RecompileNeeded( void )
{
	fileTime_t total, added;
	char **fileList, **dirList;
	uint64_t numFiles, numDirs;
	uint64_t i;

	if ( ml_alwaysCompile->i ) {
		Con_Printf( COLOR_MAGENTA "forced recompilation is enabled.\n" );
		return qtrue;
	} else if ( !m_pCacheData ) {
		Con_Printf( COLOR_MAGENTA "module code cache not loaded.\n" );
		return qtrue;
	}

	total = 0;
	for ( i = 0; i < m_nModuleCount; i++ ) {
		Con_DPrintf( "fetching checksum of '%s'...\n", m_pModList[i].info->m_szId );
		added = ModuleLib_GetDirectoryChecksum( m_pModList[i].info->m_pHandle->GetModulePath() );
		Con_DPrintf( "...got %lu\n", added );
		total += added;
	}

	Con_Printf( COLOR_MAGENTA "...Got total checksum of %lu\n", total );

	return m_pCacheData->checksum != total;
}

bool CModuleLib::LoadByteCodeCache( void )
{
	const char *path;
	uint64_t nLength;
	uint64_t i;
	asCodeCacheHeader_t *header;
	fileStats_t cacheStats, dataStats;
	bool recompiled;

	if ( ml_alwaysCompile->i ) {
		Con_Printf( "Forced recompilation is on.\n" );
		return false;
	}

	path = va( CACHE_DIR "/asmetadata.bin" );

	nLength = FS_LoadFile( path, (void **)&header );
	if ( !nLength || !header ) {
		Con_Printf( "Error opening '" CACHE_DIR "/asmetadata.bin'\n" );
		return false;
	}

	if ( nLength < sizeof( *header ) ) {
		Con_Printf( COLOR_RED "LoadByteCodeCache: metadata header too small for a valid file\n" );
		return false;
	}

	m_pCacheData = (asCodeCacheHeader_t *)Hunk_Alloc( sizeof( *header ) + ( MAX_NPATH * header->modCount ), h_high );
	memcpy( m_pCacheData, header, sizeof( *header ) + ( MAX_NPATH * header->modCount ) );

	FS_FreeFile( header );

	Con_Printf( "...Got checksum %lu\n", m_pCacheData->checksum );

	if ( ( recompiled = RecompileNeeded() ) ) {
		m_bModulesOutdated = qtrue;
		Con_Printf( COLOR_MAGENTA "...module code has been changed.\n" );

		for ( i = 0; i < m_nModuleCount; i++ ) {
			m_pModList[i].info->m_pHandle->Compile();
		}
		return false;
	} else {
		m_bModulesOutdated = qfalse;
		Con_Printf( COLOR_GREEN "...module code is up to date.\n" );
	}

	if ( m_pCacheData->gameVersion.m_nVersionMajor != _NOMAD_VERSION_MAJOR || m_pCacheData->gameVersion.m_nVersionUpdate != _NOMAD_VERSION_UPDATE
		|| m_pCacheData->gameVersion.m_nVersionPatch != _NOMAD_VERSION_PATCH )
	{
		// recompile, different version
		return false;
	}
	else {
		// load the bytecode
		CModuleCacheHandle dataStream( va( CACHE_DIR "/ascodecache.dat" ), FS_OPEN_READ );
		int ret;
		
		ret = m_pModule->LoadByteCode( &dataStream, (bool *)&m_pCacheData->hasDebugSymbols );
		/*
		if ( ret != asSUCCESS ) {
			// clean cache to get rid of any old and/or corrupt code
			FS_Remove( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ) );
			FS_HomeRemove( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ) );
			FS_Remove( va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() ) );
			FS_HomeRemove( va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() ) );
			Con_Printf( COLOR_RED "Error couldn't load cached byte code for '%s'\n", moduleName.c_str() );
			return false;
		}
		*/
		if ( ret != asSUCCESS ) {
			Con_Printf( COLOR_RED "Error couldn't load cached script code (error: %i)\n", ret );
			return false;
		}
	}

	return true;
}

qboolean CModuleLib::IsModuleInCache( const char *name ) const
{
	uint64_t i;
	char *ptr;

	if ( !m_pCacheData || m_bModulesOutdated ) {
		// failed to load
		return qfalse;
	}

	ptr = m_pCacheData->modList;
	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( !N_stricmp( ptr, name ) ) {
			return qtrue;
		}
		ptr += MAX_NPATH;
	}
	return qfalse;
}

void CModuleLib::LoadModList( void )
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

	m_pModList = (module_t *)Hunk_Alloc( sizeof( *m_pModList ) * m_nModuleCount, h_high );
	for ( i = 0; i < m_nModuleCount; i++ ) {
		m_pModList[i].info = &m_pLoadList[i];
	}

	// check for required modules
	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( !N_stricmp( m_pModList[i].info->m_szName, "nomadmain" ) || !N_stricmp( m_pModList[i].info->m_szName, "gameui" ) ) {
			m_pModList[i].isRequired = qtrue;
		}
	}

	nLength = FS_LoadFile( CACHE_DIR "/loadlist.cfg", (void **)&b );
	if ( !nLength || !b ) {
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
}

CModuleLib::CModuleLib( void )
{
	const char *path;
	char **fileList;
	uint64_t nFiles, i;
	int error;
	bool loaded, recompiled;

	if ( s_pModuleInstance && s_pModuleInstance->m_pEngine ) {
		return;
	}

	g_pModuleLib = this;

	memset( this, 0, sizeof( *this ) );

	//
	// init angelscript api
	//
	m_pEngine = dynamic_cast<asCScriptEngine *>( asCreateScriptEngine() );
	if ( !m_pEngine ) {
		N_Error( ERR_DROP, "CModuleLib::Init: failed to create an AngelScript Engine context" );
	}
	CheckASCall( m_pEngine->SetMessageCallback( asFUNCTION( Module_ASMessage_f ), NULL, asCALL_CDECL ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_ALLOW_MULTILINE_STRINGS, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_ALLOW_UNSAFE_REFERENCES, false ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT, false ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_COMPILER_WARNINGS, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_OPTIMIZE_BYTECODE, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_INCLUDE_JIT_INSTRUCTIONS, ml_allowJIT->i ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_BUILD_WITHOUT_LINE_CUES, !ml_debugMode->i ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_REQUIRE_ENUM_SCOPE, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_USE_CHARACTER_LITERALS, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_COPY_SCRIPT_SECTIONS, true ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_AUTO_GARBAGE_COLLECT, false ) );
	CheckASCall( m_pEngine->SetEngineProperty( asEP_HEREDOC_TRIM_MODE, 0 ) );

	m_pScriptBuilder = new ( Hunk_Alloc( sizeof( *m_pScriptBuilder ), h_high ) ) CScriptBuilder();
	g_pDebugger = new ( Hunk_Alloc( sizeof( *g_pDebugger ), h_high ) ) CDebugger();

	if ( ml_allowJIT->i ) {
		m_pCompiler = new ( Hunk_Alloc( sizeof( *m_pCompiler ), h_high ) ) asCJITCompiler();
		CheckASCall( m_pEngine->SetJITCompiler( m_pCompiler ) );
	}
	m_pScriptBuilder->SetIncludeCallback( Module_IncludeCallback_f, NULL );

	if ( ( error = g_pModuleLib->GetScriptBuilder()->StartNewModule( g_pModuleLib->GetScriptEngine(), "GlobalModule" ) ) != asSUCCESS ) {
		N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to start module 'GlobalModule' -- %s", AS_PrintErrorString( error ) );
	}

	m_pModule = m_pScriptBuilder->GetModule();
	Assert( m_pModule );

	m_pContext = m_pEngine->CreateContext();

	//
	// load all the modules
	//

	Con_Printf( "Loading module configurations from \"modules/\"...\n" );

	path = "modules/";
	fileList = FS_ListFiles( "modules/", "/", &nFiles );
	m_pLoadList = (CModuleInfo *)Hunk_Alloc( sizeof( *m_pLoadList ) * nFiles, h_high );

	// add standard definitions
	g_pModuleLib->AddDefaultProcs();

	for ( i = 0; i < nFiles; i++ ) {
		if ( N_streq( fileList[i], "." ) || N_streq( fileList[i], ".." ) ) {
			continue;
		}
		Con_Printf( "...found module directory '%s'\n", fileList[i] );
		LoadModule( fileList[i] );
	}
	LoadModList();

	Con_Printf( "Checking if recompilation is needed...\n" );
	loaded = LoadByteCodeCache();
	if ( ( recompiled = RecompileNeeded() ) ) {
		Con_Printf( COLOR_MAGENTA "...module code changed.\n" );
		m_bModulesOutdated = qtrue;

		for ( i = 0; i < m_nModuleCount; i++ ) {
			m_pLoadList[i].m_pHandle->Compile();
		}
	} else {
		Con_Printf( COLOR_GREEN "...module code up to date.\n" );\
		m_bModulesOutdated = qfalse;
	}

	CheckASCall( m_pEngine->SetEngineProperty( asEP_INIT_GLOBAL_VARS_AFTER_BUILD, !loaded ) );

	for ( i = 0; i < m_nModuleCount; i++ ) {
		m_pModList[i].info = &m_pLoadList[i];
	}

	FS_FreeFileList( fileList );

	/*
	// bind all the functions
	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( !m_pLoadList[i].m_pHandle->IsValid() ) { // failed to compile or didn't have any source files
			continue;
		}

		// this might fail, yes but, it'll just crash automatically if we're missing a function, and not
		// all functions have to be imported
		m_pLoadList[i].m_pHandle->IsValid()->BindAllImportedFunctions();
	}
	*/

	if ( !loaded ) {
		try {
			if ( ( error = g_pModuleLib->GetScriptBuilder()->BuildModule() ) != asSUCCESS ) {
				N_Error( ERR_DROP, "Error building GlobalModule" );
				// clean cache to get rid of any old and/or corrupt code
				Cbuf_ExecuteText( EXEC_APPEND, "ml.clean_script_cache\n" );
			}
		} catch ( const std::exception& e ) {
			Con_Printf( COLOR_RED "ERROR: std::exception thrown when compiling GlobalModule, %s\n", e.what() );
			return;
		}
	}

	if ( !loaded ) {
		// only save if we've got new stuff
		SaveByteCodeCache();
	}
	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( !m_pLoadList[i].m_pHandle->InitCalls() ) {
			Con_Printf( COLOR_YELLOW "WARNING: failed to initialize calling procs for module '%s'\n", m_pLoadList[i].m_szName );
		}
	}

	#undef CALL
}

CModuleLib::~CModuleLib() {
}

static void ML_PrintStringCache_f( void ) {
	uint64_t i;

	Con_Printf( "Module StringFactory Cache:\n" );

	auto it = g_pStringFactory->m_StringCache.cbegin();
	for ( i = 0; i < g_pStringFactory->m_StringCache.size(); i++, it++ ) {
		Con_Printf( "%8lu: 0x%08lx (%i references)", i, (uintptr_t)(void *)it->first.c_str(), it->second );
	}
}

CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion )
{
	PROFILE_FUNCTION();

	Con_Printf( "---------- InitModuleLib ----------\n" );
	Con_Printf( "Initializing mod library...\n" );

	//
	// init cvars
	//
	ml_angelScript_DebugPrint = Cvar_Get( "ml_angelScript_DebugPrint", "1", CVAR_LATCH | CVAR_PRIVATE | CVAR_SAVE );
	Cvar_SetDescription( ml_angelScript_DebugPrint, "Set to 1 if you want verbose AngelScript API logging" );
	ml_alwaysCompile = Cvar_Get( "ml_alwaysCompile", "0", CVAR_LATCH | CVAR_PRIVATE | CVAR_SAVE );
	Cvar_SetDescription( ml_alwaysCompile, "Toggle forced compilation of a module every time the game loads as opposed to using cached bytecode" );
	ml_allowJIT = Cvar_Get( "ml_allowJIT", "0", CVAR_LATCH | CVAR_PRIVATE | CVAR_SAVE );
	Cvar_SetDescription( ml_allowJIT, "Toggle JIT compilation of a module" );
	ml_debugMode = Cvar_Get( "ml_debugMode", "0", CVAR_LATCH | CVAR_TEMP );
	Cvar_SetDescription( ml_debugMode, "Set to 1 whenever a module is being debugged" );
	ml_garbageCollectionIterations = Cvar_Get( "ml_garbageCollectionIterations", "4", CVAR_TEMP | CVAR_PRIVATE );
	Cvar_SetDescription( ml_garbageCollectionIterations, "Sets the number of iterations per garbage collection loop" );

//	Cmd_AddCommand( "ml.clean_script_cache", ML_CleanCache_f );
	Cmd_AddCommand( "ml.garbage_collection_stats", ML_GarbageCollectionStats_f );
	Cmd_AddCommand( "ml_debug.print_string_cache", ML_PrintStringCache_f );

	Mem_Init();

	// FIXME: angelscript's thread manager is fucking broken on unix (stalls forever)
	asSetGlobalMemoryFunctions( AS_Alloc, AS_Free );

	g_pModuleLib = new ( Hunk_Alloc( sizeof( *g_pModuleLib ), h_high ) ) CModuleLib();

	Con_Printf( "--------------------\n" );

	return g_pModuleLib;
}

void CModuleLib::Shutdown( qboolean quit )
{
	uint64_t i, j;
	memoryStats_t allocs, frees;

	if ( m_bRecursiveShutdown ) {
		Con_Printf( COLOR_YELLOW "WARNING: CModuleLib::Shutdown (recursive)\n" );
		return;
	}
	m_bRecursiveShutdown = qtrue;

	Con_Printf( "CModuleLib::Shutdown: shutting down modules...\n" );

	if ( com_errorEntered ) {
		if ( asGetActiveContext() ) {
			Cbuf_ExecuteText( EXEC_NOW, va( "ml_debug.set_active \"%s\"", m_pCurrentHandle->GetName().c_str() ) );
			g_pDebugger->PrintCallstack( asGetActiveContext() );
		}

		// TODO: use the serializer to create a sort of coredump like file for the active script
	}

//	Cmd_RemoveCommand( "ml.clean_script_cache" );
	Cmd_RemoveCommand( "ml.garbage_collection_stats" );
	Cmd_RemoveCommand( "ml_debug.set_active" );
	Cmd_RemoveCommand( "ml_debug.print_help" );
	Cmd_RemoveCommand( "ml_debug.stacktrace" );
	Cmd_RemoveCommand( "ml_debug.backtrace" );
	Cmd_RemoveCommand( "ml_debug.clear_breakpoint" );
	Cmd_RemoveCommand( "ml_debug.set_breakpoint" );
	Cmd_RemoveCommand( "ml_debug.continue" );
	Cmd_RemoveCommand( "ml_debug.step_into" );
	Cmd_RemoveCommand( "ml_debug.step_out" );
	Cmd_RemoveCommand( "ml_debug.step_over" );
	Cmd_RemoveCommand( "ml_debug.print_array_memory_stats" );
	Cmd_RemoveCommand( "ml_debug.print_string_cache" );

	m_pEngine->GarbageCollect( asGC_FULL_CYCLE | asGC_DESTROY_GARBAGE | asGC_DETECT_GARBAGE, 100 );
	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( m_pLoadList[i].m_pHandle ) {
			m_pLoadList[i].m_pHandle->CallFunc( ModuleShutdown, 0, NULL );
			m_pLoadList[i].m_pHandle->ClearMemory();
			m_pLoadList[i].m_pHandle->~CModuleHandle();
		}
	}
	
	if ( m_bRegistered ) {
		if ( m_pCompiler ) {
			m_pCompiler->~asCJITCompiler();
		}
		m_pScriptBuilder->~CScriptBuilder();
		g_pDebugger->~CDebugger();
	}

	Mem_GetFrameStats( allocs, frees );
	Con_Printf( "\n" );
	Con_Printf( "Allocated Bytes: %li\n", allocs.totalSize );
	Con_Printf( "Total Allocations: %li\n", allocs.num );
	Con_Printf( "Biggest Allocation: %li\n", allocs.maxSize );
	Con_Printf( "Smallest Allocation: %li\n", allocs.minSize );
	Con_Printf( "Released Bytes: %li\n", frees.totalSize );
	Con_Printf( "Total Frees: %li\n", frees.num );
	Con_Printf( "Biggest Free: %li\n", frees.maxSize );
	Con_Printf( "Smallest Free: %li\n", frees.minSize );

	m_bRegistered = qfalse;
	m_bRecursiveShutdown = qfalse;
	g_pModuleLib = NULL;
	g_pDebugger = NULL;

	Z_FreeTags( TAG_MODULES );
	Mem_Shutdown();
}

CDebugger *CModuleLib::GetDebugger( void ) {
	return g_pDebugger;
}

asIScriptEngine *CModuleLib::GetScriptEngine( void ) {
	return m_pEngine;
}

CScriptBuilder *CModuleLib::GetScriptBuilder( void ) {
	return m_pScriptBuilder;
}

CContextMgr *CModuleLib::GetContextManager( void ) {
	return m_pContextManager;
}

CModuleInfo *CModuleLib::GetModule( const char *pName ) {
	PROFILE_FUNCTION();
	
	uint64_t i;

	for ( i = 0; i < m_nModuleCount; i++ ) {
		if ( !N_stricmp( m_pLoadList[i].m_szName, pName ) ) {
			return &m_pLoadList[i];
		}
	}
	return NULL;
}

CModuleInfo *CModuleLib::GetLoadList( void ) {
	return m_pLoadList;
}

uint64_t CModuleLib::GetModCount( void ) const {
	return m_nModuleCount;
}
