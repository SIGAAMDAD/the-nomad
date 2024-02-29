#include "module_public.h"
#include "module_alloc.h"
#include "module_handle.h"
#include "angelscript/as_bytecode.h"

static const char *funcNames[NumFuncs] = {
    "ModuleInit",
    "ModuleShutdown",
    "ModuleOnLevelStart",
    "ModuleOnLevelEnd",
    "ModuleOnCommand",
    "ModuleOnKeyEvent",
    "ModuleOnMouseEvent",
    "ModuleOnRunTic",
    "ModuleOnSaveGame",
    "ModuleOnLoadGame",
    "ModuleDrawConfiguration",
    "ModuleSaveConfiguration",

    "ModuleGetCurrentLevelIndex",
    "ModuleRewindToLastCheckpoint"
};

CModuleHandle::CModuleHandle( const char *pName, const UtlVector<UtlString>& sourceFiles )
    : m_szName( pName ), m_pScriptContext( NULL ), m_pScriptModule( NULL )
{
    int error;

    if ( !sourceFiles.size() ) {
        Con_Printf( COLOR_YELLOW "WARNING: no source files found for '%s', not compiling\n", pName );
        return;
    }

    if ( ( error = g_pModuleLib->GetScriptBuilder()->StartNewModule( g_pModuleLib->GetScriptEngine(), pName ) ) != asSUCCESS ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to start module '%s' -- %s", pName, AS_PrintErrorString( error ) );
    }

    // add standard definitions
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION %u", _NOMAD_VERSION ) );
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION_UPDATE %u", _NOMAD_VERSION_UPDATE ) );
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION_PATCH %u", _NOMAD_VERSION_PATCH ) );

    g_pModuleLib->AddDefaultProcs();

    m_pScriptModule = g_pModuleLib->GetScriptEngine()->GetModule( pName, asGM_CREATE_IF_NOT_EXISTS );
    if ( !m_pScriptModule ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: GetModule() failed\n" );
    }

    for ( const auto& it : sourceFiles ) {
        LoadSourceFile( it );
    }

#ifdef MODULE_USE_JIT
    for ( uint32_t i = 0; i < m_pScriptModule->GetFunctionCount(); i++ ) {
        if ( ( error = compiler->CompileFunction( m_pScriptModule->GetFunctionByIndex( i ) ) ) != asSUCCESS ) {
            N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to build module '%s' -- %s", pName, AS_PrintErrorString( error ) );
        }
    }
#else
    if ( ( error = g_pModuleLib->GetScriptBuilder()->BuildModule() ) != asSUCCESS ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to build module '%s' -- %s", pName, AS_PrintErrorString( error ) );
    }
#endif

    m_pScriptContext = g_pModuleLib->GetScriptEngine()->CreateContext();
    InitCalls();
}

CModuleHandle::~CModuleHandle() {
    ClearMemory();
}

void LogExceptionInfo( asIScriptContext *pContext )
{
    const asIScriptEngine *pEngine = pContext->GetEngine();
    const asIScriptFunction *pFunc;

    pFunc = pContext->GetExceptionFunction();

    N_Error( ERR_DROP,
        "exception was thrown on module exit ->\n"
        " Module ID: %s\n"
        " Section Name: %s\n"
        " Function: %s\n"
        " Line: %i\n"
        " Error Message: %s\n"
    , pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber(),
    pContext->GetExceptionString() );
}

int CModuleHandle::CallFunc( EModuleFuncId nCallId, uint32_t nArgs, uint32_t *pArgList )
{
    uint32_t i;
    int retn;
    
    if ( !m_pScriptContext ) {
        return -1;
    }

    Con_DPrintf( "Calling function proc '%s' at 0x%08lx... (module \"%s\")\n",
        funcNames[nCallId], (uintptr_t)(void *)m_pFuncTable[nCallId], m_szName.c_str() );

    CheckASCall( m_pScriptContext->Prepare( m_pFuncTable[nCallId] ) );
    CheckASCall( m_pScriptContext->SetExceptionCallback( asFUNCTION( LogExceptionInfo ), NULL, asCALL_CDECL ) );

    for ( i = 0; i < nArgs; i++ ) {
        CheckASCall( m_pScriptContext->SetArgDWord( i, pArgList[i] ) );
    }

    switch ( m_pScriptContext->Execute() ) {
    case asEXECUTION_ABORTED:
    case asEXECUTION_ERROR:
    case asEXECUTION_EXCEPTION:
        // something happened in there, dunno what
        LogExceptionInfo( m_pScriptContext );
        break;
    case asEXECUTION_SUSPENDED:
    case asEXECUTION_FINISHED:
    default:
        // exited successfully
        break;
    };

    retn = (int)m_pScriptContext->GetReturnWord();

    m_pScriptContext->Unprepare();

    return retn;
}

void CModuleHandle::InitCalls( void )
{
    Con_Printf( "Initializing function procs...\n" );

    memset( m_pFuncTable, 0, sizeof( m_pFuncTable ) );

    for ( uint32_t i = 0; i < NumFuncs; i++ ) {
        if ( i == ModuleGetCurrentLevelIndex && N_stricmp( m_szName.c_str(), "nomadmain" ) ) {
            break; // not sgame
        }
        Con_DPrintf( "Checking if module has function '%s'...\n", funcNames[i] );
        m_pFuncTable[i] = m_pScriptModule->GetFunctionByName( funcNames[i] );
        if ( m_pFuncTable[i] ) {
            Con_Printf( COLOR_GREEN "Module \"%s\" registered with proc '%s'.\n", m_szName.c_str(), funcNames[i] );
        } else {
            Con_Printf( COLOR_MAGENTA "Module \"%s\" not registered with proc '%s'.\n", m_szName.c_str(), funcNames[i] );
        }
    }
}

void CModuleHandle::LoadSourceFile( const UtlString& filename )
{
    union {
        void *v;
        char *b;
    } f;
    int retn;
    uint64_t length;

    length = FS_LoadFile( filename.c_str(), &f.v );
    if ( !f.v ) {
        N_Error( ERR_DROP, "CModuleHandle::LoadSourceFile: failed to load source file '%s'", filename.c_str() );
    }
    
    retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
    if ( retn < 0 ) {
        Con_Printf( COLOR_RED "ERROR: failed to add source file '%s' -- %s\n", filename.c_str(), AS_PrintErrorString( retn ) );
    }

    FS_FreeFile( f.v );
}

void CModuleHandle::SaveToCache( void ) const
{
    asIBinaryStream *dataStream;

//    dataStream = new CModuleCacheHandle;
///    m_pScriptModule->SaveByteCode( dataStream, ml_debugMode->i );
}

/*
* CModuleHandle::ClearMemory: called whenever restarting vm or exiting
*/
void CModuleHandle::ClearMemory( void )
{
    if ( !m_pScriptModule ) {
        return;
    }

    Con_Printf( "CModuleHandle::ClearMemory: clearing memory of '%s'...\n", m_szName.c_str() );

    m_pScriptModule->ResetGlobalVars();
}

asIScriptContext *CModuleHandle::GetContext( void ) {
    return m_pScriptContext;
}

asIScriptModule *CModuleHandle::GetModule( void ) {
    return m_pScriptModule;
}

const UtlString& CModuleHandle::GetName( void ) const {
    return m_szName;
}
