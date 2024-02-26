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

moduleImport_t moduleImport;

cvar_t *ml_angelScript_DebugPrint;

static CDebugger *s_pDebugger;

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
    return "<unknown error>";
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

    submodules.reserve( parse["submodules"].size() );
    for ( const auto& it : parse["submodules"] ) {
        submodules.emplace_back( eastl::move( it.get<std::string>().c_str() ) );
    }

    pHandle = new CModuleHandle( pModule, submodules );
    m_LoadList.emplace_back( CModuleInfo( parse, pHandle ) );
}

int CModuleLib::ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... )
{
    asIScriptContext *pContext;
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
    switch ( pMsg->type ) {
    case asMSGTYPE_ERROR:
        Con_Printf( COLOR_RED "ERROR: [AngelScript](%s:%i:%i) %s\n",
            pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    case asMSGTYPE_WARNING:
        Con_Printf( COLOR_YELLOW "WARNING: [AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    case asMSGTYPE_INFORMATION:
        Con_Printf( "[AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    };
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

    length = FS_LoadFile( pInclude, &f.v );
    if ( !length || !f.v ) {
        Con_Printf( COLOR_YELLOW "WARNING: failed to load include preprocessor file '%s'!\n", pInclude );
        return -1;
    }
    pBuilder->AddSectionFromMemory( pInclude, f.b, length );
    FS_FreeFile( f.v );

    Con_Printf( "Added include file '%s' to '%s'\n", pInclude, pFrom );

    (void)unused; // shut up compiler

    return 1;
}

void ModuleLib_AddDefaultProcs( void )
{
    RegisterStdString( g_pModuleLib->GetScriptEngine() );
    RegisterScriptArray( g_pModuleLib->GetScriptEngine(), true );
    RegisterScriptDictionary( g_pModuleLib->GetScriptEngine() );
    RegisterScriptMath( g_pModuleLib->GetScriptEngine() );

    ModuleLib_Register_Engine();
    ModuleLib_Register_Cvar();
    ModuleLib_Register_RenderEngine();
    ModuleLib_Register_FileSystem();
    ModuleLib_Register_SoundSystem();
}

CModuleLib::CModuleLib( void )
{
    const char *path;

    g_pModuleLib = this;

    //
    // init angelscript api
    //
    m_pEngine = asCreateScriptEngine();
    if ( !m_pEngine ) {
        N_Error( ERR_DROP, "CModuleLib::Init: failed to create an AngelScript Engine context" );
    }
    m_pEngine->SetMessageCallback( asFUNCTION( Module_ASMessage_f ), NULL, asCALL_GENERIC );
    m_pEngine->SetEngineProperty( asEP_ALLOW_MULTILINE_STRINGS, true );
    m_pEngine->SetEngineProperty( asEP_ALLOW_UNSAFE_REFERENCES, true );
    m_pEngine->SetEngineProperty( asEP_ALWAYS_IMPL_DEFAULT_CONSTRUCT, false );
    m_pEngine->SetEngineProperty( asEP_COMPILER_WARNINGS, true );
    m_pEngine->SetEngineProperty( asEP_OPTIMIZE_BYTECODE, true );
    m_pEngine->SetDefaultNamespace( "TheNomad" );

    m_pScriptBuilder = new CScriptBuilder();
//    m_pContextManager = new CContextMgr();

    ModuleLib_AddDefaultProcs();

    s_pDebugger = new CDebugger();

    g_pModuleLib->GetScriptBuilder()->SetIncludeCallback( Module_IncludeCallback_f, NULL );

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
                Con_Printf( "- found module directory \"%s\".\n", path.c_str() );

                LoadModule( path.c_str() );
            }
        }
    } catch ( const std::exception& e ) {
        N_Error( ERR_FATAL, "InitModuleLib: failed to load module directories, std::exception was thrown -> %s", e.what() );
    }
}

CModuleLib::~CModuleLib()
{
    delete m_pScriptBuilder;
//    delete m_pContextManager;
    delete s_pDebugger;
}

CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion )
{
    Con_Printf( "InitModuleLib: initializing mod library...\n" );

    //
    // init cvars
    //
    ml_angelScript_DebugPrint = Cvar_Get( "ml_angelScript_DebugPrint", "0", CVAR_LATCH | CVAR_PRIVATE );

    // init memory manager
    Mem_Init();
    asSetGlobalMemoryFunctions( asAlloc, asFree );

    g_pModuleLib = new CModuleLib();

    return g_pModuleLib;
}

void CModuleLib::Shutdown( void )
{
    Con_Printf( "CModuleLib::Shutdown: shutting down modules...\n" );

    m_pEngine->ShutDownAndRelease();
    delete g_pModuleLib;

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
        if ( !N_stricmp( it->m_szName, pName ) ) {
            return it; // THANK YOU eastl for just being a MOTHERFUCKING POINTER instead of an overcomplicated class
        }
    }
    return NULL;
}
