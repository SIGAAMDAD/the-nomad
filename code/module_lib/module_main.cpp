// module_main.cpp -- initializes the module library

#include "module_public.h"
#include "angelscript/angelscript.h"
#include "module_handle.h"
#include "debugger.h"
#include "../game/g_game.h"
#include <glm/glm.hpp>
#include <filesystem>
#include "module_renderlib.h"

moduleImport_t moduleImport;

cvar_t *ml_angelScript_DebugPrint;

static CDebugger *s_pDebugger;

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

#if EA_CHAR8_UNIQUE
	EASTL_EASTDC_API int Vsnprintf(char8_t*  EA_RESTRICT pDestination, size_t n, const char8_t*  EA_RESTRICT pFormat, va_list arguments) {
        return N_vsnprintf( pDestination, n, pFormat, arguments );
    }
#endif
#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
	EASTL_EASTDC_API int Vsnprintf(wchar_t* EA_RESTRICT pDestination, size_t n, const wchar_t* EA_RESTRICT pFormat, va_list arguments) {
        return 0; // TODO
    }
#endif
};

void CModuleLib::LoadModule( const char *pModule )
{
    CModuleHandle *pHandle;
    CModuleParse parse( va( "%s/module.txt", pModule ) );

    if ( parse.Failed() ) {
        moduleImport.Printf( PRINT_INFO, COLOR_RED "ERROR: failed to load module configuration for \"%s\"!\n", pModule );
        return;
    }

    pHandle = new CModuleHandle( pModule, parse.GetArray( "submodules" ) );
    m_LoadList.emplace_back( CModuleInfo( parse, pHandle ) );
}

int CModuleLib::ModuleCall( CModuleInfo *pModule, EModuleFuncId nCallId, uint32_t nArgs, ... )
{
    asIScriptContext *pContext;
    va_list argptr;
    uint32_t i;
    uint32_t args[16];

    if ( !pModule ) {
        moduleImport.Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid module" );
    }
    if ( nCallId >= NumFuncs ) {
        moduleImport.Error( ERR_FATAL, "CModuleLib::ModuleCall: invalid call id" );
    }

    va_start( argptr, nArgs );
    for ( i = 0; i < nArgs; i++ ) {
       args[i] = va_arg( argptr, uint32_t ); 
    }
    va_end( argptr );

    pContext = m_pContextManager->AddContext( m_pEngine, pModule->m_pHandle->GetFunction( nCallId ) );
    pContext->Execute();
}

void Module_ASMessage_f( const asSMessageInfo *pMsg, void *param )
{
    switch ( pMsg->type ) {
    case asMSGTYPE_ERROR:
        moduleImport.Printf( PRINT_INFO, COLOR_RED "ERROR: [AngelScript](%s:%i:%i) %s\n",
            pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    case asMSGTYPE_WARNING:
        moduleImport.Printf( PRINT_WARNING, "[AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
        break;
    case asMSGTYPE_INFORMATION:
        moduleImport.Printf( PRINT_INFO, "[AngelScript](%s:%i:%i) %s\n", pMsg->section, pMsg->row, pMsg->col, pMsg->message );
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

    length = moduleImport.FS_LoadFile( pInclude, &f.v );
    if ( !length || !f.v ) {
        moduleImport.Printf( PRINT_WARNING, "failed to load include preprocessor file '%s'!\n", pInclude );
        return -1;
    }
    pBuilder->AddSectionFromMemory( pInclude, f.b, length );
    moduleImport.FS_FreeFile( f.v );

    moduleImport.Printf( PRINT_INFO, "Added include file '%s' to '%s'\n", pInclude, pFrom );

    (void)unused; // shut up compiler
}

//
// c++ compatible wrappers around angelscript engine function calls
//

void ModuleLib_Printf( const eastl::string& msg ) {
    moduleImport.Printf( PRINT_INFO, "%s", msg.c_str() );
}

void ModuleLib_AddPolyToScene( nhandle_t hShader, CScriptArray *pPolyList ) {
    re.AddPolyToScene( hShader, (const polyVert_t *)pPolyList->GetBuffer(), pPolyList->GetSize() );
}

void ModuleLib_AddDefaultProcs( void )
{
    RegisterStdString_Native( g_pModuleLib->GetScriptEngine() );
    RegisterScriptArray( g_pModuleLib->GetScriptEngine(), true );
    RegisterScriptDictionary( g_pModuleLib->GetScriptEngine() );
    RegisterScriptDictionary_Native( g_pModuleLib->GetScriptEngine() );
    RegisterScriptMath_Native( g_pModuleLib->GetScriptEngine() );

    g_pModuleLib->GetScriptEngine()->RegisterTypedef( "int32", "EngineHandle" );
    g_pModuleLib->GetScriptEngine()->RegisterTypedef( "int32", "nhandle" );
    g_pModuleLib->GetScriptEngine()->RegisterTypedef( "int32", "ShaderHandle" );
    g_pModuleLib->GetScriptEngine()->RegisterTypedef( "int32", "SpriteHandle" );

    g_pModuleLib->GetScriptEngine()->RegisterEnum( "SPEntityType" );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Playr", ET_PLAYR );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Mob", ET_MOB );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Bot", ET_BOT );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Item", ET_ITEM );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Wall", ET_WALL );
    g_pModuleLib->GetScriptEngine()->RegisterEnumValue( "SPEntityType", "Weapon", ET_WEAPON );

    // render engine
    CRenderSceneRef::Register();

    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void RE_AddEntityToScene()", asFUNCTION( re.AddEntityToScene ), asCALL_CDECL );
    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void RE_AddPolyToScene( ShaderHandle, vector<PolyVert>& )", asFUNCTION( ModuleLib_AddPolyToScene ), asCALL_CDECL );
    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void RE_AddSpriteToScene( ShaderHandle, const vec3& )", asFUNCTION( re.AddSpriteToScene ), asCALL_CDECL );
    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "ShaderHandle RegisterShader( string )", asFUNCTION( re.RegisterShader ), asCALL_CDECL );
    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "SpriteHandle RegisterSprite( string, uint, uint, uint, uint )", asFUNCTION( re.RegisterSpriteSheet ), asCALL_CDECL );

    g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( "void ConsolePrint( string& fmt )", asFUNCTION( ModuleLib_Printf ), asCALL_CDECL );

    g_pModuleLib->GetScriptBuilder()->SetIncludeCallback( Module_IncludeCallback_f, NULL );
}

CModuleLib::CModuleLib( void )
{
    const char *path;

    //
    // init angelscript api
    //
    m_pEngine = asCreateScriptEngine();
    if ( !m_pEngine ) {
        moduleImport.Error( ERR_DROP, "CModuleLib::Init: failed to create an AngelScript Engine context" );
    }
    m_pEngine->SetMessageCallback( asFUNCTION( Module_ASMessage_f ), NULL, asCALL_CDECL );

    m_pScriptBuilder = new CScriptBuilder();
    m_pContextManager = new CContextMgr();

    ModuleLib_AddDefaultProcs();

    s_pDebugger = new CDebugger();

    //
    // load all the modules
    //

    moduleImport.Printf( PRINT_INFO, "Loading module configurations...\n" );

    path = va( "%s/modules/", moduleImport.Cvar_VariableString( "fs_basepath" ) );

    // this is really inefficient but it'll do for now
    for ( const auto& it : std::filesystem::directory_iterator{ path } ) {
        if ( it.is_directory() ) {
            const std::string&& path = eastl::move( it.path().string() );
            moduleImport.Printf( PRINT_INFO, "- found module directory \"%s\".\n", path.c_str() );

            LoadModule( path.c_str() );
        }
    }
}

CModuleLib::~CModuleLib()
{
    delete m_pScriptBuilder;
    delete m_pContextManager;
    delete s_pDebugger;
}

CModuleLib *InitModuleLib( const moduleImport_t *pImport, const renderExport_t *pExport, version_t nGameVersion )
{
    memcpy( &moduleImport, pImport, sizeof(*pImport) );

    moduleImport.Printf( PRINT_INFO, "InitModuleLib: initializing mod library...\n" );

    //
    // init cvars
    //
    ml_angelScript_DebugPrint = moduleImport.Cvar_Get( "ml_angelScript_DebugPrint", "0", CVAR_LATCH | CVAR_PRIVATE );

    // init memory manager
    Mem_Init();
    asSetGlobalMemoryFunctions( asAlloc, asFree );

    g_pModuleLib = new CModuleLib();

    return g_pModuleLib;
}

void CModuleLib::Shutdown( void )
{
    moduleImport.Printf( PRINT_INFO, "CModuleLib::Shutdown: shutting down modules...\n" );

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
        if ( !N_stricmp( it->m_szName.c_str(), pName ) ) {
            return it; // THANK YOU eastl for just being a MOTHERFUCKING POINTER instead of an overcomplicated class
        }
    }
    return NULL;
}
