#include "module_proc_def.h"

#define REGISTER_ENUM_TYPE( name ) \
    CheckASCall( pEngine->RegisterEnum( name ) )
#define REGISTER_ENUM_VALUE( type, decl, value ) \
    CheckASCall( pEngine->RegisterEnumValue( type, decl, value ) )
#define REGISTER_GLOBAL_VAR( decl, ptr ) \
    CheckASCall( pEngine->RegisterGlobalProperty( decl, (void *)ptr ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
    CheckASCall( pEngine->RegisterGlobalFunction( decl, asFUNCTION( funcPtr ), asCALL_GENERIC ) )
#define REGISTER_CLASS_METHOD( className, decl, funcPtr, params, retn ) \
    CheckASCall( pEngine->RegisterObjectMethod( #className, decl, asMETHODPR( className, funcPtr, params, retn ), asCALL_GENERIC ) )
#define REGISTER_CLASS_TYPE( className, size, flags ) \
    CheckASCall( pEngine->RegisterObjectType( #className, size, asGetTypeTraits<className>() | flags ) )
#define REGISTER_CLASS_PROPERTY( className, decl, offset ) \
    CheckASCall( pEngine->RegisterObjectProperty( className, decl, offset ) )
#define REGISTER_CLASS_BEHAVE( className, behave, decl, funcPtr, params, retn ) \
    CheckASCall( pEngine->RegisterObjectBehaviour( #className, behave, decl, \
        asMETHODPR( className, funcPtr, params, retn ), asCALL_GENERIC ) )

#define SET_NAMESPACE( name ) \
    CheckASCall( pEngine->SetDefaultNamespace( name ) )


static const nhandle_t script_FS_INVALID_HANDLE = FS_INVALID_HANDLE;

static void ModuleLib_Register_Globals( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
    g_pModuleLib->GetScriptBuilder()->DefineWord( "FS_INVALID_HANDLE -1" );
}

static void ModuleLib_Register_FileSystem( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();

    SET_NAMESPACE( "TheNomad::Engine::FileSystem" );
}

static void ModuleLib_Register_Cvar( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

static void ModuleLib_Register_Cmd( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

static void ModuleLib_Register_Game( void )
{
    asIScriptEngine *pEngine = g_pModuleLib->GetScriptEngine();
}

void ModuleLib_Register_Engine( void )
{

}
