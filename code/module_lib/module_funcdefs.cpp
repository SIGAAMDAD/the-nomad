#include "module_public.h"
#include <glm/gtc/type_ptr.hpp>

//
// c++ compatible wrappers around angelscript engine function calls
//

#define REQUIRE_ARG_COUNT( amount ) \
    Assert( pGeneric->GetArgCount() == amount )
#define DEFINE_CALLBACK( name ) \
    static void ModuleLib_##name ( asIScriptGeneric *pGeneric )
#define GETVAR( type, index ) \
    *(const type *)pGeneric->GetArgAddress( index )

#define REGISTER_ENUM_TYPE( name ) \
    ValidateEnumType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnum( name ) )
#define REGISTER_ENUM_VALUE( type, name, value ) \
    ValidateEnumValue( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterEnumValue( type, name, value ) )
#define REGISTER_METHOD_FUNCTION( obj, decl, funcPtr ) \
    ValidateMethod( __func__, decl, g_pModuleLib->GetScriptEngine()->RegisterObjectMethod( obj, decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_GLOBAL_FUNCTION( decl, funcPtr ) \
    ValidateFunction( __func__, decl,\
        g_pModuleLib->GetScriptEngine()->RegisterGlobalFunction( decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_OBJECT_TYPE( name, obj, traits ) \
    ValidateObjectType( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterObjectType( name, sizeof(obj), traits | asGetTypeTraits<obj>() ) )
#define REGISTER_OBJECT_PROPERTY( obj, var, offset ) \
    ValidateObjectProperty( __func__, #var, g_pModuleLib->GetScriptEngine()->RegisterObjectProperty( obj, #var, offset ) )
#define REGISTER_OBJECT_BEHAVIOUR( obj, type, decl, funcPtr ) \
    ValidateObjectBehaviour( __func__, #type, g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( obj, type, decl, asFUNCTION( funcPtr ), asCALL_GENERIC ) )
#define REGISTER_TYPEDEF( type, alias ) \
    ValidateTypedef( __func__, alias, g_pModuleLib->GetScriptEngine()->RegisterTypedef( type, alias ) )

#define SET_NAMESPACE( name ) g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( name )
#define RESET_NAMESPACE() g_pModuleLib->GetScriptEngine()->SetDefaultNamespace( "" )

static bool Add( const char *name ) {
    for ( const auto& it : g_pModuleLib->m_RegisteredProcs ) {
        if ( !N_strcmp( name, it.c_str() ) ) {
            return false;
        }
    }
    g_pModuleLib->m_RegisteredProcs.emplace_back( name );
    return true;
}

static void ValidateEnumType( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register enumeration type %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateEnumValue( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register enumeration value %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateFunction( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register global function %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateMethod( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register method function %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectType( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object type %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectBehaviour( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object behaviour %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateObjectProperty( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register object property %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

static void ValidateTypedef( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register typedef %s -- %s", func, name, AS_PrintErrorString( result ) );
    }
}

DEFINE_CALLBACK( CvarRegisterGeneric ) {
    vmCvar_t vmCvar;

    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *value = (string_t *)pGeneric->GetArgObject( 1 );
    asDWORD flags = pGeneric->GetArgDWord( 2 );
    asINT64 *intValue = (asINT64 *)pGeneric->GetArgAddress( 3 );
    float *floatValue = (float *)pGeneric->GetArgAddress( 4 );
    asINT32 *modificationCount = (asINT32 *)pGeneric->GetArgAddress( 5 );
    cvarHandle_t *cvarHandle = (cvarHandle_t *)pGeneric->GetArgAddress( 6 );

    Cvar_Register( &vmCvar, name->c_str(), value->c_str(), flags, CVAR_PRIVATE );

    *value = (const char *)vmCvar.s;
    *intValue = vmCvar.i;
    *floatValue = vmCvar.f;
    *modificationCount = vmCvar.modificationCount;
    *cvarHandle = vmCvar.handle;
}

DEFINE_CALLBACK( CvarUpdateGeneric ) {
    vmCvar_t vmCvar;

    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *value = (string_t *)pGeneric->GetArgObject( 1 );
    asINT64 *intValue = (asINT64 *)pGeneric->GetArgAddress( 2 );
    float *floatValue = (float *)pGeneric->GetArgAddress( 3 );
    asINT32 *modificationCount = (asINT32 *)pGeneric->GetArgAddress( 4 );
    const cvarHandle_t cvarHandle = pGeneric->GetArgWord( 5 );

    memset( &vmCvar, 0, sizeof(vmCvar) );
    N_strncpyz( vmCvar.s, value->c_str(), sizeof(vmCvar.s) );
    vmCvar.i = *intValue;
    vmCvar.f = *floatValue;
    vmCvar.modificationCount = *modificationCount;
    vmCvar.handle = cvarHandle;
    Cvar_Update( &vmCvar, CVAR_PRIVATE );

    *value = (const char *)vmCvar.s;
    *intValue = vmCvar.i;
    *floatValue = vmCvar.f;
    *modificationCount = vmCvar.modificationCount;
}

DEFINE_CALLBACK( CvarSetValueGeneric ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const string_t *value = (const string_t *)pGeneric->GetArgObject( 1 );
    Cvar_Set( name->c_str(), value->c_str() );
}

void ModuleLib_Register_Cvar( void )
{
    SET_NAMESPACE( "TheNomad" );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarRegister( const string&, string&, uint, int64&, float&, int&, int& )", CvarRegisterGeneric );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarSet( const string&, const string& )", CvarSetValueGeneric );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarUpdate( const string&, string&, int64&, float&, int&, const int )", CvarUpdateGeneric );
    RESET_NAMESPACE();
}

DEFINE_CALLBACK( Snd_PlaySfx ) {
    REQUIRE_ARG_COUNT( 1 );
    Snd_PlaySfx( pGeneric->GetArgWord( 0 ) );
}

DEFINE_CALLBACK( Snd_SetLoopingTrack ) {
    REQUIRE_ARG_COUNT( 1 );
    Snd_SetLoopingTrack( pGeneric->GetArgWord( 0 ) );
}

DEFINE_CALLBACK( Snd_ClearLoopingTrack ) {
    Snd_ClearLoopingTrack();
}

DEFINE_CALLBACK( Snd_RegisterSfx ) {
    REQUIRE_ARG_COUNT( 1 );
    pGeneric->SetReturnWord( Snd_RegisterSfx( (const char *)pGeneric->GetArgAddress( 0 ) ) );
}

DEFINE_CALLBACK( Snd_RegisterTrack ) {
    REQUIRE_ARG_COUNT( 1 );
    pGeneric->SetReturnWord( Snd_RegisterTrack( (const char *)pGeneric->GetArgAddress( 0 ) ) );
}

void ModuleLib_Register_SoundSystem( void )
{
    SET_NAMESPACE( "TheNomad::Engine::SoundSystem" );

    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::PlaySfx( int )", Snd_PlaySfx );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetLoopingTrack( int )", Snd_SetLoopingTrack );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::ClearLoopingTrack()", Snd_ClearLoopingTrack );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterSfx( const string& )", Snd_RegisterSfx );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterTrack( const string& )", Snd_RegisterTrack );

    RESET_NAMESPACE();
}

void ModuleLib_Register_FileSystem( void )
{
    SET_NAMESPACE( "TheNomad::Engine::FileSystem" );

    RESET_NAMESPACE();
}

DEFINE_CALLBACK( GetMapData ) {
    REQUIRE_ARG_COUNT( 3 );
    CScriptArray *tileData = (CScriptArray *)pGeneric->GetArgAddress( 0 );
    CScriptArray *soundBits = (CScriptArray *)pGeneric->GetArgAddress( 1 );
}

DEFINE_CALLBACK( BeginSaveSection ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    g_pArchiveHandler->BeginSaveSection( name->c_str() );
}

DEFINE_CALLBACK( EndSaveSection ) {
    g_pArchiveHandler->EndSaveSection();
}

DEFINE_CALLBACK( SaveInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT8 arg = *(const asINT8 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveChar( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT16 arg = *(const asINT16 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT32 arg = *(const asINT32 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT64 arg = *(const asINT64 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveLong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const byte arg = *(const byte *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveByte( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT16 arg = *(const asUINT16 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT32 arg = *(const asUINT32 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT64 arg = *(const asUINT64 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveULong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveFloat ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const float arg = pGeneric->GetArgFloat( 1 );
    g_pArchiveHandler->SaveFloat( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveString ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const string_t *arg = (const string_t *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveString( name->c_str(), arg->c_str() );
}

DEFINE_CALLBACK( SaveVec2 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec2 v = *(const glm::vec2 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec2( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( SaveVec3 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec3 v = *(const glm::vec3 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec3( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( SaveVec4 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec4 v = *(const glm::vec4 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec4( name->c_str(), glm::value_ptr( v ) );
}


DEFINE_CALLBACK( LoadInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT8 arg = *(const asINT8 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveChar( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT16 arg = *(const asINT16 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveShort( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT32 arg = *(const asINT32 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveInt( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asINT64 arg = *(const asINT64 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveLong( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const byte arg = *(const byte *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveByte( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT16 arg = *(const asUINT16 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUShort( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT32 arg = *(const asUINT32 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUInt( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const asUINT64 arg = *(const asUINT64 *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveULong( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadFloat ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const float arg = pGeneric->GetArgFloat( 1 );
    g_pArchiveHandler->SaveFloat( name->c_str(), arg );
}

DEFINE_CALLBACK( LoadString ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *arg = (const string_t *)pGeneric->GetArgObject( 1 );

    arg->resize( MAX_STRING_CHARS );
    memset( arg->data(), 0, MAX_STRING_CHARS );

    g_pArchiveHandler->LoadString( name->c_str(), str->data(), MAX_STRING_CHARS, pGeneric->GetArgWord( 3 ) );
}

DEFINE_CALLBACK( LoadVec2 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec2 v = *(const glm::vec2 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec2( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( LoadVec3 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec3 v = *(const glm::vec3 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec3( name->c_str(), glm::value_ptr( v ) );
}

DEFINE_CALLBACK( LoadVec4 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const glm::vec4 v = *(const glm::vec4 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->SaveVec4( name->c_str(), glm::value_ptr( v ) );
}

void ModuleLib_Register_Game( void )
{
    SET_NAMESPACE( "TheNomad::GameSystem" );

    REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::BeginSaveSection( const string& )", BeginSaveSection );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::GameSystem::EndSaveSection()", EndSaveSection );

    // these are here because if they change in the map editor or the engine, it'll break sgame
    REGISTER_ENUM_TYPE( "TheNomad::GameSystem::EntityType" );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Playr", ET_PLAYR );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Mob", ET_MOB );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Bot", ET_BOT );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Item", ET_ITEM );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Wall", ET_WALL );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::EntityType", "Weapon", ET_WEAPON );

    REGISTER_ENUM_TYPE( "TheNomad::GameSystem::DirType" );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "North", DIR_NORTH );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "NorthEast", DIR_NORTH_EAST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "East", DIR_EAST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "SouthEast", DIR_SOUTH_EAST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "South", DIR_SOUTH );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "SouthWest", DIR_SOUTH_WEST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "West", DIR_WEST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "NorthWest", DIR_NORTH_WEST );
    REGISTER_ENUM_VALUE( "TheNomad::GameSystem::DirType", "Inside", DIR_NULL );

    REGISTER_GLOBAL_FUNCTION( "int TheNomad::GameSystem::GetMapData( array<uint32>&, array<int32>&, array<vec3>&, int&, int& )", GetMapData );

    RESET_NAMESPACE();
}

DEFINE_CALLBACK( ConstructVec3Generic ) {

}

DEFINE_CALLBACK( CopyConstructVec3Generic ) {

}

static void ModuleLib_Register_Math( void ) {
}

DEFINE_CALLBACK( ConsolePrint ) {
    REQUIRE_ARG_COUNT( 1 );
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    Con_Printf( "%s", msg->c_str() );
}

DEFINE_CALLBACK( GameError ) {
    REQUIRE_ARG_COUNT( 1 );
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    N_Error( ERR_DROP, "%s", msg->c_str() );
}

void ModuleLib_Register_Engine( void )
{
    SET_NAMESPACE( "TheNomad" );
    SET_NAMESPACE( "TheNomad::Engine" );
    RESET_NAMESPACE();

    REGISTER_GLOBAL_FUNCTION( "void ConsolePrint( const string& )", ConsolePrint );
    REGISTER_GLOBAL_FUNCTION( "void GameError( const string& )", GameError );
}

DEFINE_CALLBACK( RenderScene ) {
    REQUIRE_ARG_COUNT( 6 );
    renderSceneRef_t refdef;
    memset( &refdef, 0, sizeof(refdef) );
    refdef.x = pGeneric->GetArgDWord( 0 );
    refdef.y = pGeneric->GetArgDWord( 1 );
    refdef.width = pGeneric->GetArgDWord( 2 );
    refdef.height = pGeneric->GetArgDWord( 3 );
    refdef.flags = pGeneric->GetArgDWord( 4 );
    refdef.time = pGeneric->GetArgDWord( 5 );
    re.RenderScene( &refdef );
}

DEFINE_CALLBACK( AddEntityToScene ) {
    REQUIRE_ARG_COUNT( 3 );
    refEntity_t refEntity;
    memset( &refEntity, 0, sizeof(refEntity) );
    refEntity.hShader = pGeneric->GetArgWord( 0 );
    VectorCopy( refEntity.origin, (float *)pGeneric->GetArgAddress( 1 ) );
    refEntity.flags = pGeneric->GetArgDWord( 2 );
}

DEFINE_CALLBACK( AddPolyToScene ) {
    REQUIRE_ARG_COUNT( 2 );
    CScriptArray *pPolyList = (CScriptArray *)pGeneric->GetArgAddress( 1 );
    re.AddPolyToScene( pGeneric->GetArgWord( 0 ), (const polyVert_t *)pPolyList->GetBuffer(), pPolyList->GetSize() );
}

DEFINE_CALLBACK( AddSpriteToScene ) {
    REQUIRE_ARG_COUNT( 3 );
    re.AddSpriteToScene( (const vec_t *)pGeneric->GetArgAddress( 0 ), pGeneric->GetArgWord( 1 ), pGeneric->GetArgWord( 2 ) );
}

DEFINE_CALLBACK( RegisterShader ) {
    REQUIRE_ARG_COUNT( 1 );
    pGeneric->SetReturnWord( re.RegisterShader( ( (const string_t *)pGeneric->GetArgAddress( 0 ) )->c_str() ) );
}

DEFINE_CALLBACK( RegisterSpriteSheet ) {
    REQUIRE_ARG_COUNT( 5 );
    const string_t *npath = (const string_t *)pGeneric->GetArgAddress( 0 );

    pGeneric->SetReturnWord( re.RegisterSpriteSheet( npath->c_str(), pGeneric->GetArgDWord( 1 ), pGeneric->GetArgDWord( 2 ),
        pGeneric->GetArgDWord( 3 ), pGeneric->GetArgDWord( 4 ) ) );
}

void ModuleLib_Register_RenderEngine( void )
{
    SET_NAMESPACE( "TheNomad::Engine::Renderer" );

    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_RenderScene( uint, uint, uint, uint, uint, uint )", RenderScene );
//    REGISTER_GLOBAL_FUNCTION( "void RE_AddEntityToScene( ShaderHandle, vec3, uint,  )", AddEntityToScene );
//    REGISTER_GLOBAL_FUNCTION( "void RE_AddPolyToScene( ShaderHandle, const array<PolyVert>@ )", AddPolyToScene );
    REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddSpriteToScene( float, float, float, int, int )", AddSpriteToScene );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterShader( const string& )", RegisterShader );
    REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RegisterSprite( const string&, uint, uint, uint, uint )", RegisterSpriteSheet );

    RESET_NAMESPACE();
}
