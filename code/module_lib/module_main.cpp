// module_main.cpp -- initializes the module library

#include "module_public.h"
#include "angelscript/angelscript.h"
#include "module_handle.h"
#include "debugger.h"
#include "../game/g_game.h"
#include <glm/glm.hpp>
#include <filesystem>
#include "module_renderlib.h"
#include "module_funcdefs.hpp"
#include "module_stringfactory.hpp"

moduleImport_t moduleImport;

cvar_t *ml_angelScript_DebugPrint;
cvar_t *ml_debugMode;
cvar_t *ml_alwaysCompile;
cvar_t *ml_allowJIT;
cvar_t *ml_garbageCollectionIterations;

static CDebugger *s_pDebugger;

static void ML_CleanCache_f( void ) {
    const char *path;

    Con_Printf( "Clearing cached bytecode script files...\n" );
    for ( auto it : g_pModuleLib->GetLoadList() ) {
        path = va( "_cache/%s_code.dat", it->m_pHandle->GetName().c_str() );
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
        return "all imported functions couldn't be bound";
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
    CModuleHandle *pHandle;
    nlohmann::json parse;
    UtlVector<UtlString> submodules;
    CModuleInfo *m;
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
            parse = nlohmann::json::parse( f.b );
        }
        FS_FreeFile( f.v );
        
    } catch ( const nlohmann::json::exception& e ) {
        Con_Printf( COLOR_RED "ERROR: failed to load module configuration for \"%s\"!\n\tid: %i\n\tmessage: %s\n", pModule, e.id, e.what() );
        FS_FreeFile( f.v );
        return;
    }

    //
    // validate its an actual config
    //
    if ( !parse.contains( "submodules" ) ) {
        Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no array 'submodules'\n", pModule );
        return;
    }
    if ( !parse.contains( "dependencies" ) ) {
        Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no array 'depedencies'\n", pModule );
        return;
    }
    if ( parse.contains( "version" ) ) {
        if ( !parse.at( "version" ).contains( "game_version_major" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'game_version_major'\n", pModule );
            return;
        }
        if ( !parse.at( "version" ).contains( "game_version_update" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'game_version_update'\n", pModule );
            return;
        }
        if ( !parse.at( "version" ).contains( "game_version_patch" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'game_version_patch'\n", pModule );
            return;
        }
        if ( !parse.at( "version" ).contains( "version_major" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'version_major'\n", pModule );
            return;
        }
        if ( !parse.at( "version" ).contains( "version_update" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'version_update'\n", pModule );
            return;
        }
        if ( !parse.at( "version" ).contains( "version_patch" ) ) {
            Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no value for 'version_patch'\n", pModule );
            return;
        }
    } else {
        Con_Printf( COLOR_RED "ERROR: invalid module configuration for \"%s\", no object 'version'\n", pModule );
        return;
    }

    submodules.reserve( parse[ "submodules" ].size() );
    for ( const auto& it : parse[ "submodules" ] ) {
        submodules.emplace_back( eastl::move( it.get<std::string>().c_str() ) );
    }

    const int32_t versionMajor = parse[ "version" ][ "version_major" ];
    const int32_t versionUpdate = parse[ "version" ][ "version_update" ];
    const int32_t versionPatch = parse[ "version" ][ "version_patch" ];

    pHandle = new ( Hunk_Alloc( sizeof( *pHandle ), h_high ) ) CModuleHandle( pModule, submodules, versionMajor, versionUpdate, versionPatch );
    m = new ( Hunk_Alloc( sizeof( *m ), h_high ) ) CModuleInfo( parse, pHandle );
    m_LoadList.emplace_back( m );
}

void CModuleLib::RunModules( EModuleFuncId nCallId, uint32_t nArgs, ... )
{
    va_list argptr;
    uint64_t j;
    uint32_t i;
    uint32_t args[16];

    if ( nCallId >= NumFuncs ) {
        N_Error( ERR_FATAL, "CModuleLib::RunModules: invalid call id" );
    }
    
    va_start( argptr, nArgs );
    for ( i = 0; i < nArgs; i++ ) {
        args[i] = va_arg( argptr, uint32_t );
    }
    va_end( argptr );

    switch ( nCallId ) {
    case ModuleOnLevelStart: {
    case ModuleOnLevelEnd:
        CTimer time;

        time.Run();
        Con_DPrintf( "Garbage collection started...\n" );
        g_pModuleLib->GetScriptEngine()->GarbageCollect( 1, (uint32_t)ml_garbageCollectionIterations->i );
        time.Stop();
        Con_DPrintf( "Garbage collection: %llims, %li iterations\n", time.ElapsedMilliseconds().count(), ml_garbageCollectionIterations->i );
        break; }
    };

    for ( j = 0; j < m_LoadList.size(); j++ ) {
        if ( m_LoadList[i] == sgvm ) {
            continue; // avoid running it twice
        }
        if ( m_LoadList[j]->m_pHandle->CallFunc( nCallId, nArgs, args ) == -1 ) {
            Con_Printf( COLOR_YELLOW "WARNING: module \"%s\" returned error code on call to proc \"%s\".\n",
                m_LoadList[j]->m_szName, funcDefs[ nCallId ].name );
        }
    }
}

int CModuleLib::ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... )
{
    va_list argptr;
    uint32_t i;
    uint32_t args[16];

    if ( !pModule ) {
        N_Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid module" );
    }
    if ( nCallId >= NumFuncs ) {
        N_Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid call id" );
    }
    
    va_start( argptr, nArgs );
    for ( i = 0; i < nArgs; i++ ) {
        args[i] = va_arg( argptr, uint32_t );
    }
    va_end( argptr );

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
    case asMSGTYPE_INFORMATION:
        if ( !ml_angelScript_DebugPrint->i ) {
            return;
        }
        Con_Printf( "[AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    };

    if ( error ) {
        //Sys_MessageBox( "*** ERROR DURING MODULE COMPILATION ***",
        //    va( "An error occurred during the compilation of a module section\n"
        //        "%s:%i:%i %s"
        //    , pMsg->section, pMsg->row, pMsg->col, pMsg->message ),
        //    false );
    }
}

void *asAlloc( size_t nBytes ) {
    return Mem_Alloc( nBytes );
}

void asFree( void *pBuffer ) {
    Mem_Free( pBuffer );
}

int Module_IncludeCallback_f( const char *pInclude, const char *pFrom, CScriptBuilder *pBuilder, void *unused )
{
    union {
        void *v;
        char *b;
    } f;
    uint64_t length;
    const char *path;

    path = va( "%s%s", g_pModuleLib->GetCurrentHandle()->GetModulePath(), pInclude );
    length = FS_LoadFile( path, &f.v );
    if ( !length || !f.v ) {
        g_pModuleLib->GetScriptEngine()->WriteMessage(
            g_pModuleLib->GetCurrentHandle()->GetModulePath(),
            0, 0, asMSGTYPE_WARNING, va( "failed to load include preprocessor file \"%s\"", path ) );
        return -1;
    }
    pBuilder->AddSectionFromMemory( COM_SkipPath( const_cast<char *>( pInclude ) ), f.b, length );
    FS_FreeFile( f.v );

    Con_Printf( "Added include file '%s' to '%s'\n", path, pFrom );

    (void)unused; // shut up compiler

    return 1;
}

bool CModuleLib::AddDefaultProcs( void ) const {
    if ( m_bRegistered ) {
        return true;
    }

    RegisterScriptHandle( m_pEngine );
    RegisterStdString( m_pEngine );
    RegisterScriptArray( m_pEngine, true );
    RegisterScriptDictionary( m_pEngine );
    RegisterScriptMath( m_pEngine );

    ModuleLib_Register_Engine();
    const_cast<CModuleLib *>( this )->m_bRegistered = qtrue;

    return true;
}

CModuleLib::CModuleLib( void )
{
    const char *path;
    int ret;

    g_pModuleLib = this;
    memset( this, 0, sizeof( *this ) );

    //
    // init angelscript api
    //
    m_pEngine = asCreateScriptEngine();
    if ( !m_pEngine ) {
        N_Error( ERR_DROP, "CModuleLib::Init: failed to create an AngelScript Engine context" );
    }
    CheckASCall( m_pEngine->SetMessageCallback( asFUNCTION( Module_ASMessage_f ), NULL, asCALL_CDECL ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_ALLOW_MULTILINE_STRINGS, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_ALLOW_UNSAFE_REFERENCES, false ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT, false ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_COMPILER_WARNINGS, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_OPTIMIZE_BYTECODE, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_INCLUDE_JIT_INSTRUCTIONS, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_REQUIRE_ENUM_SCOPE, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_USE_CHARACTER_LITERALS, true ) );
    CheckASCall( m_pEngine->SetEngineProperty( asEP_AUTO_GARBAGE_COLLECT, false ) );

    m_pScriptBuilder = new ( Hunk_Alloc( sizeof( *m_pScriptBuilder ), h_high ) ) CScriptBuilder();
    s_pDebugger = new ( Hunk_Alloc( sizeof( *s_pDebugger ), h_high ) ) CDebugger();

    if ( ml_allowJIT->i ) {
        m_pCompiler = new ( Hunk_Alloc( sizeof( *m_pCompiler ), h_high ) ) asCJITCompiler();
        CheckASCall( m_pEngine->SetJITCompiler( m_pCompiler ) );
    }
    m_pScriptBuilder->SetIncludeCallback( Module_IncludeCallback_f, NULL );

    //
    // load all the modules
    //

    Con_Printf( "Loading module configurations from \"%s\"...\n", Cvar_VariableString( "fs_basegame" ) );

    path = va( "%s/modules/", Cvar_VariableString( "fs_basegame" ) );

    try {
        // this is really inefficient but it'll do for now
        for ( const auto& it : std::filesystem::directory_iterator{ path } ) {
            if ( it.is_directory() ) {
                const std::string path = it.path().filename().string();
                Con_Printf( "...found module directory \"%s\".\n", path.c_str() );

                LoadModule( path.c_str() );
            }
        }
    } catch ( const std::exception& e ) {
        N_Error( ERR_FATAL, "InitModuleLib: failed to load module directories, std::exception was thrown -> %s", e.what() );
    }

    #undef CALL
}

CModuleLib::~CModuleLib()
{
    m_pEngine->ShutDownAndRelease();
    asFree( m_pEngine );

    s_pDebugger = NULL;
    g_pStringFactory = NULL;
}

CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion )
{
    Con_Printf( "---------- InitModuleLib ----------\n" );
    Con_Printf( "Initializing mod library...\n" );

    //
    // init cvars
    //
    ml_angelScript_DebugPrint = Cvar_Get( "ml_angelScript_DebugPrint", "1", CVAR_LATCH | CVAR_PRIVATE | CVAR_SAVE );
    Cvar_SetDescription( ml_angelScript_DebugPrint, "Set to 1 if you want verbose AngelScript API logging" );
    ml_alwaysCompile = Cvar_Get( "ml_alwaysCompile", "0", CVAR_INIT | CVAR_PRIVATE | CVAR_SAVE );
    Cvar_SetDescription( ml_alwaysCompile, "Toggle forced compilation of a module every time the game loads as opposed to using cached bytecode" );
    ml_allowJIT = Cvar_Get( "ml_allowJIT", "1", CVAR_LATCH | CVAR_PRIVATE | CVAR_SAVE );
    Cvar_SetDescription( ml_allowJIT, "Toggle JIT compilation of a module" );
    ml_debugMode = Cvar_Get( "ml_debugMode", "0", CVAR_LATCH | CVAR_TEMP );
    Cvar_SetDescription( ml_debugMode, "Set to 1 whenever a module is being debugged" );
    ml_garbageCollectionIterations = Cvar_Get( "ml_garbageCollectionIterations", "4", CVAR_LATCH | CVAR_TEMP | CVAR_PRIVATE );
    Cvar_SetDescription( ml_garbageCollectionIterations, "Sets the number of iterations per garbage collection loop" );

    Cmd_AddCommand( "ml.clean_script_cache", ML_CleanCache_f );
    Cmd_AddCommand( "ml.garbage_collection_stats", ML_GarbageCollectionStats_f );

    // init memory manager
    Mem_Init();
    asSetGlobalMemoryFunctions( asAlloc, asFree );

    g_pModuleLib = new ( Hunk_Alloc( sizeof( *g_pModuleLib ), h_high ) ) CModuleLib();

    Con_Printf( "--------------------\n" );

    return g_pModuleLib;
}

void CModuleLib::Shutdown( void )
{
    Con_Printf( "CModuleLib::Shutdown: shutting down modules...\n" );

    g_pModuleLib->~CModuleLib();
    g_pModuleLib = NULL;
    Mem_Shutdown();
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
    for ( auto it = m_LoadList.begin(); it != m_LoadList.end(); it++ ) {
        if ( !N_stricmp( ( *it )->m_szName, pName ) ) {
            return *it; // THANK YOU eastl for just being a MOTHERFUCKING POINTER instead of an overcomplicated class
        }
    }
    return NULL;
}

void CModuleLib::RegisterCvar( const UtlString& name, const UtlString& value, uint32_t flags, bool trackChanges, uint32_t privateFlag )
{
    vmCvar_t vmCvar;

    const auto it = m_CvarList.find( name.c_str() );
    if ( it != m_CvarList.cend() ) {
        Con_Printf( "CModuleLib::RegisterCvar: vmCvar '%s' already registered.\n", name.c_str() );
        return;
    }

    memset( &vmCvar, 0, sizeof(vmCvar) );
    Cvar_Register( &vmCvar, name.c_str(), value.c_str(), flags, privateFlag );
    m_CvarList.emplace( name.c_str(), vmCvar );

    Con_Printf( "Registered VM CVar \"%s\" with default value of \"%s\"\n", name.c_str(), value.c_str() );
}

UtlVector<CModuleInfo *>& CModuleLib::GetLoadList( void ) {
    return m_LoadList;
}
