#include "module_public.h"
#include "module_handle.h"
#include "angelscript/as_bytecode.h"

CModuleHandle::CModuleHandle( const char *pName, const UtlVector<UtlString>& sourceFiles )
    : m_szName( pName ), m_pScriptContext( NULL ), m_pScriptModule( NULL )
{
    memset( m_pFuncTable, 0, sizeof(m_pFuncTable) );

    if ( !sourceFiles.size() ) {
        Con_Printf( COLOR_YELLOW "WARNING: no source files found for '%s', not compiling\n", pName );
        return;
    }

    g_pModuleLib->GetScriptBuilder()->StartNewModule( g_pModuleLib->GetScriptEngine(), pName );

    // add standard definitions
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION %u", _NOMAD_VERSION ) );
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION_UPDATE %u", _NOMAD_VERSION_UPDATE ) );
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define NOMAD_VERSION_PATCH %u", _NOMAD_VERSION_PATCH ) );

    m_pScriptModule = g_pModuleLib->GetScriptEngine()->GetModule( pName, asGM_ALWAYS_CREATE );

    for ( const auto& it : sourceFiles ) {
        LoadSourceFile( it );
    }

    if ( g_pModuleLib->GetScriptBuilder()->BuildModule() != 0 ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to build module '%s'", pName );
    }

    m_pScriptContext = g_pModuleLib->GetScriptEngine()->CreateContext();
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
    , pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), pContext->GetExceptionLineNumber() );
}

int CModuleHandle::CallFunc( EModuleFuncId nCallId, uint32_t nArgs, uint32_t *pArgList )
{
    uint32_t i;

    if ( !m_pScriptContext ) {
        return -1;
    }

    m_pScriptContext->Prepare( m_pFuncTable[nCallId] );
    m_pScriptContext->SetExceptionCallback( asFUNCTION( LogExceptionInfo ), NULL, asCALL_CDECL );

    for ( i = 0; i < nArgs; i++ ) {
        m_pScriptContext->SetArgDWord( i, pArgList[i] );
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

    return (int)m_pScriptContext->GetReturnWord();
}

void CModuleHandle::InitCalls( void )
{
    m_pFuncTable[ModuleInit] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_Init" );
    m_pFuncTable[ModuleShutdown] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_Shutdown" );
    m_pFuncTable[ModuleOnLevelEnd] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnLevelStart" );
    m_pFuncTable[ModuleOnLevelStart] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnLevelEnd" );
    m_pFuncTable[ModuleCommandLine] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_Command" );
    m_pFuncTable[ModuleOnKeyEvent] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnKeyEvent" );
    m_pFuncTable[ModuleOnMouseEvent] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnMouseEvent" );
    m_pFuncTable[ModuleOnRunTic] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_RunTic" );
    m_pFuncTable[ModuleOnSaveGame] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnSaveGame" );
    m_pFuncTable[ModuleOnLoadGame] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_OnLoadGame" );
    m_pFuncTable[ModuleDrawConfiguration] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_DrawConfiguration" );
    m_pFuncTable[ModuleSaveConfiguration] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "Module_SaveConfiguration" );

    if ( !N_stricmp( m_szName.c_str(), "nomadmain" ) ) {
        m_pFuncTable[ModuleGetCurrentLevelIndex] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "GetCurrentLevelIndex" );
        m_pFuncTable[ModuleRewindToLastCheckpoint] = g_pModuleLib->GetScriptEngine()->GetModule( m_szName.c_str() )->GetFunctionByDecl( "RewindToLastCheckpoint" );
    }
}

void CModuleHandle::LoadSourceFile( const UtlString& filename )
{
    union {
        void *v;
        char *b;
    } f;
    uint64_t length;

    length = FS_LoadFile( filename.c_str(), &f.v );
    if ( !f.v ) {
        N_Error( ERR_DROP, "CModuleHandle::LoadSourceFile: failed to load source file '%s'", filename.c_str() );
    }
    g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
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
