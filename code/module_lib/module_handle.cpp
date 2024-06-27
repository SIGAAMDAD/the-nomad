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
    /*
    { "int Init()", ModuleInit, 0, qtrue },
    { "int Shutdown()", ModuleShutdown, 0, qtrue },
    { "int OnConsoleCommand()", ModuleCommandLine, 0, qfalse },
    { "int DrawConfiguration()", ModuleDrawConfiguration, 0, qfalse },
    { "int SaveConfiguration()", ModuleSaveConfiguration, 0, qfalse },
    { "int OnKeyEvent( uint, uint )", ModuleOnKeyEvent, 2, qfalse },
    { "int OnMouseEvent( int, int )", ModuleOnMouseEvent, 2, qfalse },
    { "int OnLevelStart()", ModuleOnLevelStart, 0, qfalse },
    { "int OnLevelEnd()", ModuleOnLevelEnd, 0, qfalse },
    { "int OnRunTic( uint )", ModuleOnRunTic, 1, qtrue },
    { "int OnSaveGame()", ModuleOnSaveGame, 0, qfalse },
    { "int OnLoadGame()", ModuleOnLoadGame, 0, qfalse }
    */
    { "int ModuleOnInit()", ModuleInit, 0, qtrue },
    { "int ModuleOnShutdown()", ModuleShutdown, 0, qtrue },
    { "int ModuleOnConsoleCommand()", ModuleCommandLine, 0, qfalse },
    { "int ModuleDrawConfiguration()", ModuleDrawConfiguration, 0, qfalse },
    { "int ModuleSaveConfiguration()", ModuleSaveConfiguration, 0, qfalse },
    { "int ModuleOnKeyEvent( int, int )", ModuleOnKeyEvent, 2, qfalse },
    { "int ModuleOnMouseEvent( int, int )", ModuleOnMouseEvent, 2, qfalse },
    { "int ModuleOnLevelStart()", ModuleOnLevelStart, 0, qfalse },
    { "int ModuleOnLevelEnd()", ModuleOnLevelEnd, 0, qfalse },
    { "int ModuleOnRunTic( int )", ModuleOnRunTic, 1, qtrue },
    { "int ModuleOnSaveGame()", ModuleOnSaveGame, 0, qfalse },
    { "int ModuleOnLoadGame()", ModuleOnLoadGame, 0, qfalse },
    { "int ModuleOnJoystickEvent( int, int, int, int, int, int )", ModuleOnJoystickEvent, 6, qfalse }
};

CModuleHandle::CModuleHandle( const char *pName, const char *pDescription, const nlohmann::json& sourceFiles, int32_t moduleVersionMajor,
    int32_t moduleVersionUpdate, int32_t moduleVersionPatch, const nlohmann::json& includePaths
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

    if ( ( error = g_pModuleLib->GetScriptBuilder()->StartNewModule( g_pModuleLib->GetScriptEngine(), pName ) ) != asSUCCESS ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: failed to start module '%s' -- %s", pName, AS_PrintErrorString( error ) );
    }

    g_pModuleLib->SetHandle( this );
    m_pScriptModule = g_pModuleLib->GetScriptEngine()->GetModule( pName, asGM_CREATE_IF_NOT_EXISTS );
    m_IncludePaths = eastl::move( includePaths );
    m_SourceFiles = eastl::move( sourceFiles );
    
    if ( !m_pScriptModule ) {
        N_Error( ERR_DROP, "CModuleHandle::CModuleHandle: GetModule() failed on \"%s\"\n", pName );
    }

    // add standard definitions
    g_pModuleLib->AddDefaultProcs();
    
    switch ( ( error = LoadFromCache() ) ) {
    case 1:
        Con_Printf( "Loaded module bytecode from cache.\n" );
        break;
    case 0:
        Con_Printf( "forced recomplation option is on.\n" );
        break;
    case -1:
        Con_Printf( COLOR_RED "failed to load module bytecode.\n" );
        break;
    };

    N_strncpyz( name, m_szName.c_str(), sizeof( name ) - 1 );
    g_pModuleLib->GetScriptBuilder()->DefineWord( va( "MODULE_%s", N_strupr( name ) ) );
    
    if ( error != 1 ) {
        Build( sourceFiles );
    }
    
    m_pScriptContext = g_pModuleLib->GetScriptEngine()->CreateContext();
    if ( !InitCalls() ) {
        return;
    }
    m_pScriptContext->AddRef();

//    m_pScriptModule->SetUserData( this );
    SaveToCache();

    m_nLastCallId = NumFuncs;
    m_bLoaded = qtrue;
}

CModuleHandle::~CModuleHandle() {
    ClearMemory();
}

void CModuleHandle::Build( const nlohmann::json& sourceFiles ) {
    int error;

    for ( const auto& it : sourceFiles ) {
        if ( !LoadSourceFile( it ) ) {
            N_Error( ERR_DROP, "CModuleHandle::Build: failed to compile source file '%s'", it.get<string_t>().c_str() );
        }
    }

    try {
        if ( ( error = g_pModuleLib->GetScriptBuilder()->BuildModule() ) != asSUCCESS ) {
            Con_Printf( COLOR_RED "ERROR: CModuleHandle::CModuleHandle: failed to build module '%s' -- %s\n", m_szName.c_str(), AS_PrintErrorString( error ) );

            // clean cache to get rid of any old and/or corrupt code
            Cbuf_ExecuteText( EXEC_APPEND, "ml.clean_scripCACHE_DIR t\n" );
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
    
    if ( !m_pScriptContext ) {
        return -1;
    }

    PROFILE_BLOCK_BEGIN( va( "module '%s'", m_szName.c_str() ) );

    if ( !m_pFuncTable[ nCallId ] ) {
        return 1; // just wasn't registered, any required func procs will have thrown an error in init stage
    }

    CheckASCall( m_pScriptContext->SetExceptionCallback( asFUNCTION( LogExceptionInfo ), this, asCALL_CDECL ) );

    // prevent a nested call infinite recursion
    if ( m_nLastCallId == nCallId ) {
        N_Error( ERR_DROP, "CModuleHandle::CallFunc: recursive nested module call" );
    }
    m_nLastCallId = nCallId;

    if ( m_pScriptContext->GetState() == asEXECUTION_ACTIVE ) {
        // we're running something right now, so push a new state and
        // call into that instead
        CheckASCall( m_pScriptContext->PushState() );
        CheckASCall( m_pScriptContext->Prepare( m_pFuncTable[ nCallId ] ) );

        if ( ml_debugMode->i && g_pDebugger->m_pModule && g_pDebugger->m_pModule->m_pHandle == this ) {
            CheckASCall( m_pScriptContext->SetLineCallback( asMETHOD( CDebugger, LineCallback ), g_pDebugger, asCALL_THISCALL ) );
        }
    
        g_pModuleLib->SetHandle( this );
    
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
            , pFunc->GetModuleName(), pFunc->GetScriptSectionName(), pFunc->GetDeclaration(), m_pScriptContext->GetExceptionLineNumber(),
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

        CheckASCall( m_pScriptContext->PopState() );

        return retn;
    }
    CheckASCall( m_pScriptContext->Prepare( m_pFuncTable[ nCallId ] ) );

    g_pModuleLib->GetScriptEngine()->GarbageCollect( asGC_DETECT_GARBAGE, 1 );

    if ( ml_debugMode->i && g_pDebugger->m_pModule && g_pDebugger->m_pModule->m_pHandle == this ) {
        CheckASCall( m_pScriptContext->SetLineCallback( asMETHOD( CDebugger, LineCallback ), g_pDebugger, asCALL_THISCALL ) );
    }

    g_pModuleLib->SetHandle( this );

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
            " Error MEssage: %s\n"
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

    m_nLastCallId = NumFuncs;

    return retn;
}

void CModuleHandle::RegisterGameObject( void )
{
}

bool CModuleHandle::InitCalls( void )
{
//    asIScriptFunction *pFactory;
    uint32_t i;

    Con_Printf( "Initializing function procs...\n" );

/*
    m_pModuleObject = m_pScriptModule->GetTypeInfoByDecl( "ModuleObject" );
    if ( !m_pModuleObject ) {
        Con_Printf( COLOR_RED "Module \"%s\" not registered with required AngelScript Object 'ModuleObject'\n", m_szName.c_str() );
        return false;
    }

    pFactory = m_pModuleObject->GetFactoryByDecl( "ModuleObject@ ModuleObject()" );
    if ( !pFactory ) {
        Con_Printf( COLOR_RED "Module \"%s\" has ModuleObject class registered but no default factory\n", m_szName.c_str() );
        return false;
    }

    CheckASCall( m_pScriptContext->Prepare( pFactory ) );
    CheckASCall( m_pScriptContext->Execute() );

    m_pEntryPoint = *(asIScriptObject **)m_pScriptContext->GetAddressOfReturnValue();
    AssertMsg( m_pEntryPoint, "Failed to create a ModuleObject instance!" );
*/

    memset( m_pFuncTable, 0, sizeof( m_pFuncTable ) );

    for ( i = 0; i < NumFuncs; i++ ) {
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
            Con_Printf( COLOR_RED "Module \"%s\" has proc '%s' but not the correct args (%u).\n", m_szName.c_str(), funcDefs[i].name,
                funcDefs[i].expectedArgs );
            return false;
        }
    }
    return true;
}

/*
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
*/

bool CModuleHandle::LoadSourceFile( const string_t& filename )
{
    union {
        void *v;
        char *b;
    } f;
    int retn, errCount;
    const char *path;
    uint64_t length;
    Preprocessor preprocessor;
    Preprocessor::FileSource fileSource;
    UtlString data;
    UtlVector<char> out;

    path = va( "modules/%s/%s", m_szName.c_str(), filename.c_str() );
    length = FS_LoadFile( path, &f.v );
    if ( !f.v ) {
        Con_Printf( "Couldn't load script file '%s'.\n", path );
        return false;
    }

    data.resize( length );
    memcpy( data.data(), f.v, length );

    if ( ( errCount = preprocessor.preprocess( filename.c_str(), data, fileSource, out ) ) > 0 ) {
        retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), f.b, length );
        if ( retn < 0 ) {
            Con_Printf( COLOR_RED "ERROR: failed to compile source file '%s' -- %s, %i errors\n", filename.c_str(),
                AS_PrintErrorString( retn ), errCount );
            return false;
        }
    } else {
        retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), out.data(), out.size() );
        if ( retn < 0 ) {
            Con_Printf( COLOR_RED "ERROR: failed to compile source file '%s' -- %s\n", filename.c_str(), AS_PrintErrorString( retn ) );
            return false;
        }
    }
    FS_FreeFile( f.v );
    /*
    int retn;
    fileHandle_t f;
    Preprocessor macroManager;
    Preprocessor::FileSource source;
    eastl::string data;
    UtlVector<char> out;

    f = FS_FOpenRead( va( "modules/%s/%s", m_szName.c_str(), filename.c_str() ) );
    if ( f == FS_INVALID_HANDLE ) {
        N_Error( ERR_DROP, "CModuleHandle::LoadSourceFile: failed to load source file '%s'", filename.c_str() );
    }
    data.resize( FS_FileLength( f ) );
    if ( !FS_Read( data.data(), data.size(), f ) ) {
        N_Error( ERR_DROP, "CModuleHandle::LoadSourceFile: failed to load source file '%s'", filename.c_str() );
    }
    FS_FClose( f );

    macroManager.preprocess( eastl::move( filename.c_str() ), data, source, out );
    
    retn = g_pModuleLib->GetScriptBuilder()->AddSectionFromMemory( filename.c_str(), out.data(), out.size() );
    if ( retn < 0 ) {
        Con_Printf( COLOR_RED "ERROR: failed to add source file '%s' -- %s\n", filename.c_str(), AS_PrintErrorString( retn ) );
    }
    */

    return true;
}

#define AS_CACHE_CODE_IDENT (('C'<<24)+('B'<<16)+('S'<<8)+'A')

typedef struct {
    int64_t ident;
    version_t gameVersion;
    int32_t moduleVersionMajor;
    int32_t moduleVersionUpdate;
    int32_t moduleVersionPatch;
    uint32_t checksum;
    qboolean hasDebugSymbols;
} asCodeCacheHeader_t;

static void SaveCodeDataCache( const string_t& moduleName, const CModuleHandle *pHandle, asIScriptModule *pModule )
{
    asCodeCacheHeader_t header;
    int ret;
    CModuleCacheHandle dataStream( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ), FS_OPEN_WRITE );
    byte *pByteCode;
    uint64_t nLength;
    fileHandle_t hFile;

    memset( &header, 0, sizeof( header ) );
    header.hasDebugSymbols = ml_debugMode->i;
    header.gameVersion.m_nVersionMajor = _NOMAD_VERSION;
    header.gameVersion.m_nVersionUpdate = _NOMAD_VERSION_UPDATE;
    header.gameVersion.m_nVersionPatch = _NOMAD_VERSION_PATCH;
    header.ident = AS_CACHE_CODE_IDENT;
    pHandle->GetVersion( &header.moduleVersionMajor, &header.moduleVersionUpdate, &header.moduleVersionPatch );

    ret = pModule->SaveByteCode( &dataStream, header.hasDebugSymbols );
    if ( ret != asSUCCESS ) {
        Con_Printf( COLOR_RED "ERROR: failed to save module bytecode for '%s'\n", moduleName.c_str() );
    }
    FS_FClose( dataStream.m_hFile );

    FS_WriteFile( va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() ), &header, sizeof( header ) );
}

static bool LoadCodeFromCache( const string_t& moduleName, const CModuleHandle *pHandle, asIScriptModule *pModule )
{
    const char *path;
    uint64_t nLength;
    asCodeCacheHeader_t *header;
    uint32_t checksum;
    byte *pByteCode;
    int32_t versionMajor, versionUpdate, versionPatch;
    uint64_t i;

    path = va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() );
    nLength = FS_LoadFile( path, (void **)&header );
    if ( !nLength || !header ) {
        return false;
    }

    if ( nLength != sizeof( *header ) ) {
        Con_Printf( COLOR_RED "LoadCodeFromCache: module script metadata for '%s' has a bad header\n", moduleName.c_str() );
        return false;
    }

    pHandle->GetVersion( &versionMajor, &versionUpdate, &versionPatch );

    //if ( header->checksum != checksum ) {
    //    // recompile, updated checksum
    //    Con_Printf( "Module script code '%s' changed, recompiling.\n", moduleName.c_str() );
    //    return false;
    //}
    if ( header->moduleVersionMajor != versionMajor || header->moduleVersionUpdate != versionUpdate
        || header->moduleVersionPatch != versionPatch )
    {
        // recompile, different version
        return false;
    }
    else {
        // load the bytecode
        CModuleCacheHandle dataStream( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ), FS_OPEN_READ );
        int ret;
        
        ret = pModule->LoadByteCode( &dataStream, (bool *)&header->hasDebugSymbols );
        if ( ret != asSUCCESS ) {
            // clean cache to get rid of any old and/or corrupt code
            FS_Remove( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ) );
            FS_HomeRemove( va( CACHE_DIR "/%s_code.dat", moduleName.c_str() ) );
            FS_Remove( va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() ) );
            FS_HomeRemove( va( CACHE_DIR "/%s_metadata.bin", moduleName.c_str() ) );
            Con_Printf( COLOR_RED "Error couldn't load cached byte code for '%s'\n", moduleName.c_str() );
            return false;
        }
    }

    FS_FreeFile( header );

    return true;
}

void CModuleHandle::SaveToCache( void ) const {
    PROFILE_FUNCTION();
    
    Con_Printf( "Saving compiled module \"%s\" bytecode...\n", m_szName.c_str() );

    SaveCodeDataCache( m_szName, this, m_pScriptModule );
}

int CModuleHandle::LoadFromCache( void ) {
    PROFILE_FUNCTION();

    if ( ml_alwaysCompile->i ) {
        return 0; // force recompilation
    }

    Con_Printf( "Loading compiled module \"%s\" bytecode...\n", m_szName.c_str() );

    if ( !LoadCodeFromCache( m_szName, this, m_pScriptModule ) ) {
        return -1; // just recompile it
    }

    return 1;
}

/*
* CModuleHandle::ClearMemory: called whenever exiting
*/
void CModuleHandle::ClearMemory( void )
{
    uint64_t i;

    if ( !m_pScriptModule || !m_pScriptContext ) {
        return;
    }

    if ( m_pScriptContext->GetState() == asCONTEXT_ACTIVE ) {
        m_pScriptContext->Abort();
    }

    Con_Printf( "CModuleHandle::ClearMemory: clearing memory of '%s'...\n", m_szName.c_str() );

    for ( i = 0; i < NumFuncs; i++ ) {
        if ( m_pFuncTable[i] ) {
            m_pFuncTable[i]->Release();
        }
    }

//    m_pEntryPoint->Release();
//    m_pModuleObject->Release();

    m_pScriptModule->UnbindAllImportedFunctions();
    CheckASCall( m_pScriptContext->Unprepare() );

    m_pScriptModule = NULL;
    m_pScriptContext = NULL;
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
