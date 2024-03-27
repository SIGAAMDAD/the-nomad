#include "module_public.h"
#include "module_alloc.h"
#include "module_handle.h"
#include "angelscript/as_bytecode.h"
#include "module_funcdefs.hpp"
#include "module_debugger.h"

const moduleFunc_t funcDefs[NumFuncs] = {
    { "int ModuleInit()", ModuleInit, 0, qtrue },
    { "int ModuleOnConsoleCommand()", ModuleCommandLine, 0, qfalse },
    { "int ModuleDrawConfiguration()", ModuleDrawConfiguration, 0, qfalse },
    { "int ModuleSaveConfiguration()", ModuleSaveConfiguration, 0, qfalse },
    { "int ModuleOnKeyEvent( uint, uint )", ModuleOnKeyEvent, 2, qfalse },
    { "int ModuleOnMouseEvent( int, int )", ModuleOnMouseEvent, 2, qfalse },
    { "int ModuleOnLevelStart()", ModuleOnLevelStart, 0, qfalse },
    { "int ModuleOnLevelEnd()", ModuleOnLevelEnd, 0, qfalse },
    { "int ModuleOnRunTic( uint )", ModuleOnRunTic, 1, qtrue },
    { "int ModuleOnSaveGame()", ModuleOnSaveGame, 0, qfalse },
    { "int ModuleOnLoadGame()", ModuleOnLoadGame, 0, qfalse }
};

CModuleHandle::CModuleHandle( const char *pName, const std::vector<std::string>& sourceFiles, int32_t moduleVersionMajor,
    int32_t moduleVersionUpdate, int32_t moduleVersionPatch, const std::vector<std::string>& includePaths
)
    : m_szName( pName ), m_pScriptContext( NULL ), m_pScriptModule( NULL ), m_nVersionMajor{ moduleVersionMajor },
    m_nVersionUpdate{ moduleVersionUpdate }, m_nVersionPatch{ moduleVersionPatch }
{
    int error;

    PROFILE_BLOCK_BEGIN( "Compile Module" );

    m_bLoaded = qfalse;

    if ( !sourceFiles.size() ) {
        Con_Printf( COLOR_YELLOW "WARNING: no source files found for '%s', not compiling\n", pName );
        return;
    }

    if ( ( error = g_pModuleLib->GetScriptBuilder()->StartNewModule( g_pModuleLib->GetScriptEngine(), pName ) ) != asSUCCESS ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to start module '%s' -- %s", pName, AS_PrintErrorString( error ) );
    }
    
    // add standard definitions
    g_pModuleLib->SetHandle( this );
    g_pModuleLib->AddDefaultProcs();
    m_pScriptModule = g_pModuleLib->GetScriptEngine()->GetModule( pName, asGM_CREATE_IF_NOT_EXISTS );
    m_IncludePaths = eastl::move( includePaths );
    
    if ( !m_pScriptModule ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: GetModule() failed on \"%s\"\n", pName );
    }
    
    switch ( ( error = LoadFromCache() ) ) {
    case 1:
        Con_Printf( "Loaded module bytecode from cache.\n" );
        break;
    case 0:
        Con_Printf( "forced recomplation option is on.\n" );
        break;
    case -1:
        Con_Printf( COLOR_RED "failed to load module bytecode.\n" );

        // if we have any old or outdated bytecode, we'll want to clean it out
        Cbuf_ExecuteText( EXEC_APPEND, "ml.clean_script_cache\n" );
        break;
    };
    
    if ( error != 1 ) {
        Build( sourceFiles );
    }
    
    m_pScriptContext = g_pModuleLib->GetScriptEngine()->CreateContext();
    if ( !InitCalls() ) {
        return;
    }

    m_pScriptModule->SetUserData( this );
    SaveToCache();

    m_bLoaded = qtrue;
}

CModuleHandle::~CModuleHandle() {
    ClearMemory();
}

void CModuleHandle::Build( const std::vector<std::string>& sourceFiles ) {
    int error;

    for ( const auto& it : sourceFiles ) {
        LoadSourceFile( it );
    }

    try {
        if ( ( error = g_pModuleLib->GetScriptBuilder()->BuildModule() ) != asSUCCESS ) {
            Con_Printf( COLOR_RED "ERROR: CModuleHandle::CModuleHandle: failed to build module '%s' -- %s\n", m_szName.c_str(), AS_PrintErrorString( error ) );

            // clean cache to get rid of any old and/or corrupt code
            Cbuf_ExecuteText( EXEC_APPEND, "ml.clean_script_cache\n" );
        }
    } catch ( const std::exception& e ) {
        Con_Printf( COLOR_RED "ERROR: std::exception thrown when loading module \"%s\", %s\n", m_szName.c_str(), e.what() );
        return;
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

    pFunc = pContext->GetExceptionFunction();
    pHandle = (const CModuleHandle *)userData;

    Con_Printf( COLOR_RED "ERROR: exception thrown by module, printing a stacktrace...\n" );
    Cbuf_ExecuteText( EXEC_NOW, va( "ml_debug.set_active \"%s\"", pHandle->GetName().c_str() ) );
    g_pDebugger->PrintCallstack( pContext );

    Com_snprintf( msg, sizeof( msg ) - 1,
        "exception was thrown in module ->\n"
        " Module ID: %s\n"
        " Section Name: %s\n"
        " Function: %s\n"
        " Line: %i\n"
        " Error Message: %s\n"
    , pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
    pContext->GetExceptionString() );

    N_Error( ERR_DROP, "%s", msg );
}

int CModuleHandle::CallFunc( EModuleFuncId nCallId, uint32_t nArgs, uint32_t *pArgList )
{
    uint32_t i;
    int retn;
    
    if ( !m_pScriptContext ) {
        return -1;
    }

    PROFILE_BLOCK_BEGIN( va( "module '%s'", m_szName.c_str() ) );

    CheckASCall( m_pScriptContext->Prepare( m_pFuncTable[nCallId] ) );
    CheckASCall( m_pScriptContext->SetExceptionCallback( asFUNCTION( LogExceptionInfo ), this, asCALL_CDECL ) );

    if ( ml_debugMode->i && g_pDebugger->m_pModule->m_pHandle == this ) {
        CheckASCall( m_pScriptContext->SetLineCallback( asMETHOD( CDebugger, LineCallback ), g_pDebugger, asCALL_THISCALL ) );
    }

    for ( i = 0; i < nArgs; i++ ) {
        CheckASCall( m_pScriptContext->SetArgDWord( i, pArgList[i] ) );
    }
    
    try {
        retn = m_pScriptContext->Execute();
    } catch ( const ModuleException& e ) {
        LogExceptionInfo( m_pScriptContext, this );
    } catch ( const nlohmann::json::exception& e ) {
        const asIScriptFunction *pFunc;
        pFunc = m_pScriptContext->GetExceptionFunction();
    
        N_Error( ERR_DROP,
            "nlohmann::json::exception was thrown in module ->\n"
            " Module ID: %s\n"
            " Section Name: %s\n"
            " Function: %s\n"
            " Line: %i\n"
            " Error Message: %s\n"
            " Id: %i\n"
        ,  pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), m_pScriptContext->GetExceptionLineNumber(),
        e.what(), e.id );
    }

    switch ( retn ) {
    case asEXECUTION_ABORTED:
    case asEXECUTION_ERROR:
    case asEXECUTION_EXCEPTION:
        // something happened in there, dunno what
        LogExceptionInfo( m_pScriptContext, this );
        break;
    case asEXECUTION_SUSPENDED:
    case asEXECUTION_FINISHED:
    default:
        // exited successfully
        break;
    };

    retn = (int)m_pScriptContext->GetReturnWord();

    PROFILE_BLOCK_END;

    return retn;
}

bool CModuleHandle::InitCalls( void )
{
    Con_Printf( "Initializing function procs...\n" );

    memset( m_pFuncTable, 0, sizeof( m_pFuncTable ) );

    for ( uint32_t i = 0; i < NumFuncs; i++ ) {
        Con_DPrintf( "Checking if module has function '%s'...\n", funcDefs[i].name );
        m_pFuncTable[i] = m_pScriptModule->GetFunctionByDecl( funcDefs[i].name );
        if ( m_pFuncTable[i] ) {
            Con_Printf( COLOR_GREEN "Module \"%s\" registered with proc '%s'.\n", m_szName.c_str(), funcDefs[i].name );
        } else {
            if ( funcDefs[i].required ) {
                Con_Printf( COLOR_RED "Module \"%s\" not registered with required proc '%s'.\n", m_szName.c_str(), funcDefs[i].name );
                return false;
            }
            Con_Printf( COLOR_MAGENTA "Module \"%s\" not registered with proc '%s'.\n", m_szName.c_str(), funcDefs[i].name );
            continue;
        }

        if ( m_pFuncTable[i]->GetParamCount() != funcDefs[i].expectedArgs ) {
            Con_Printf( COLOR_RED "Module \"%s\" has proc '%s' but not the correct args (%u).\n", m_szName.c_str(), funcDefs[i].name, funcDefs[i].expectedArgs );
            return false;
        }
    }
    return true;
}

void CModuleHandle::LoadSourceFile( const std::string& filename )
{
    union {
        void *v;
        char *b;
    } f;
    int retn;
    uint64_t length;

    length = FS_LoadFile( va( "modules/%s/%s", m_szName.c_str(), filename.c_str() ), &f.v );
    if ( !f.v ) {
        N_Error( ERR_DROP, "CModuleHandle::LoadSourceFile: failed to load source file '%s'", filename.c_str() );
    }
    
    retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
    if ( retn < 0 ) {
        Con_Printf( COLOR_RED "ERROR: failed to add source file '%s' -- %s\n", filename.c_str(), AS_PrintErrorString( retn ) );
    }

    FS_FreeFile( f.v );
}

#define AS_CACHE_CODE_IDENT (('C'<<24)+('B'<<16)+('S'<<8)+'A')

typedef struct {
    int64_t ident;
    version_t gameVersion;
    int32_t moduleVersion; // versionMajor
    qboolean hasDebugSymbols;
} asCodeCacheHeader_t;

void CModuleHandle::SaveToCache( void ) const {
    PROFILE_FUNCTION();
    
    CModuleCacheHandle *dataStream;
    const char *path;
    asCodeCacheHeader_t header;
    int ret;

    path = va( "_cache/%s_code.dat", m_szName.c_str() );

    dataStream = CreateStackObject( CModuleCacheHandle, path, FS_OPEN_WRITE );
    Con_Printf( "Saving compiled module \"%s\" bytecode to \"%s\"...\n", m_szName.c_str(), path );

/*
    memset( &header, 0, sizeof(header) );
    header.ident = AS_CACHE_CODE_IDENT;
    header.gameVersion = version_t{ NOMAD_VERSION_FULL };
    header.moduleVersion = m_nVersion;
#ifdef _NOMAD_DEBUG
    header.hasDebugSymbols = true;
#else
    header.hasDebugSymbols = ml_debugMode->i;
#endif

    dataStream->Write( &header, sizeof(header) ); */

    ret = m_pScriptModule->SaveByteCode( dataStream, !header.hasDebugSymbols );
    if ( ret != asSUCCESS ) {
        Con_Printf( COLOR_RED "ERROR: failed to save module bytecode.\n" );
    }
}

int CModuleHandle::LoadFromCache( void ) {
    PROFILE_FUNCTION();

    CModuleCacheHandle *dataStream;
    const char *path;
    asCodeCacheHeader_t header;
    int ret;

    if ( ml_alwaysCompile->i ) {
        return 0; // force recompilation
    }

    path = va( "_cache/%s_code.dat", m_szName.c_str() );
    if ( !FS_FileExists( path ) ) {
        return -1;
    }

    dataStream = CreateStackObject( CModuleCacheHandle, path, FS_OPEN_READ );
    Con_Printf( "Loading compiled module \"%s\" bytecode from \"%s\"...\n", m_szName.c_str(), path );

/*    dataStream->Read( &header, sizeof(header) );
    if ( header.ident != AS_CACHE_CODE_IDENT ) {
        Con_Printf( COLOR_RED "ERROR: invalid code cache file \"%s\", identifier is incorrect.\n", path );
        return -1;
    }
    if ( header.gameVersion != version_t{ NOMAD_VERSION_FULL } ) {
        Con_Printf( COLOR_YELLOW
            "WARNING: game version found in code cache file is different,"
            "GDR Games is not responsible for an unstable experience.\n" );
    }
    if ( header.moduleVersion != m_nVersion ) {
        Con_Printf( "Module version found in code cache file isn't the same as the one loaded, recompiling.\n" );
        return -1;
    }*/

    ret = m_pScriptModule->LoadByteCode( dataStream, NULL );
    if ( ret != asSUCCESS ) {
        // clean cache to get rid of any old and/or corrupt code
        Cbuf_ExecuteText( EXEC_APPEND, "ml.clean_script_cache\n" );
        
        return -1; // just recompile it
    }

    return 1;
}

/*
* CModuleHandle::ClearMemory: called whenever restarting vm or exiting
*/
void CModuleHandle::ClearMemory( void )
{
    if ( !m_pScriptModule ) {
        return;
    }

    if ( m_pScriptContext->GetState() == asCONTEXT_ACTIVE ) {
        m_pScriptContext->Abort();
    }

    Con_Printf( "CModuleHandle::ClearMemory: clearing memory of '%s'...\n", m_szName.c_str() );
    for ( uint64_t i = 0; i < NumFuncs; i++ ) {
        if ( m_pFuncTable[i] ) {
            m_pFuncTable[i]->Release();
        }
    }

    CheckASCall( g_pModuleLib->GetScriptEngine()->DiscardModule( m_pScriptModule->GetName() ) );
    CheckASCall( m_pScriptContext->Unprepare() );
    CheckASCall( m_pScriptContext->Release() );
}

asIScriptContext *CModuleHandle::GetContext( void ) {
    return m_pScriptContext;
}

asIScriptModule *CModuleHandle::GetModule( void ) {
    return m_pScriptModule;
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
