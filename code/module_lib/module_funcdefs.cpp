#include "module_public.h"
#include <glm/gtc/type_ptr.hpp>
#include <limits.h>

//
// c++ compatible wrappers around angelscript engine function calls
//

#define REQUIRE_ARG_COUNT( amount ) \
    Assert( pGeneric->GetArgCount() == amount )
#define DEFINE_CALLBACK( name ) \
    static void ModuleLib_##name ( asIScriptGeneric *pGeneric )
#define GETVAR( type, index ) \
    *(const type *)pGeneric->GetArgAddress( index )

#define REGISTER_GLOBAL_VAR( name, addr ) \
    ValidateGlobalVar( __func__, name, g_pModuleLib->GetScriptEngine()->RegisterGlobalProperty( name, (void *)addr ) )
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
    ValidateObjectBehaviour( __func__, #type, g_pModuleLib->GetScriptEngine()->RegisterObjectBehaviour( obj, type, decl, asFUNCTION( ModuleLib_##funcPtr ), asCALL_GENERIC ) )
#define REGISTER_TYPEDEF( type, alias ) \
    ValidateTypedef( __func__, alias, g_pModuleLib->GetScriptEngine()->RegisterTypedef( alias, type ) )

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

static void ValidateGlobalVar( const char *func, const char *name, int result ) {
    if ( !Add( name ) ) {
        return;
    }
    if ( result < 0 ) {
        N_Error( ERR_DROP, "%s: failed to register global variable %s -- %s", func, name, AS_PrintErrorString( result ) );
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
    const int8_t arg = *(const int8_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveChar( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int16_t arg = *(const int16_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int32_t arg = *(const int32_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const int64_t arg = *(const int64_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveLong( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint8_t arg = *(const uint8_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveByte( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint16_t arg = *(const uint16_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUShort( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint32_t arg = *(const uint32_t *)pGeneric->GetArgAddress( 1 );
    g_pArchiveHandler->SaveUInt( name->c_str(), arg );
}

DEFINE_CALLBACK( SaveUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    const uint64_t arg = *(const uint64_t *)pGeneric->GetArgAddress( 1 );
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
    g_pArchiveHandler->SaveString( name->c_str(), arg );
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

DEFINE_CALLBACK( FindSaveSection ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->GetSection( name->c_str() );
}

DEFINE_CALLBACK( LoadInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadChar( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadShort( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadInt( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(int64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadLong( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt8 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint8_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadByte( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt16 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint16_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUShort( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt32 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint32_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadUInt( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadUInt64 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadULong( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadFloat ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    *(float *)pGeneric->GetAddressOfReturnLocation() = g_pArchiveHandler->LoadFloat( name->c_str(), *(int32_t *)pGeneric->GetArgAddress( 1 ) );
}

DEFINE_CALLBACK( LoadString ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    string_t *arg = (string_t *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadString( name->c_str(), arg, *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadArray ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    CScriptArray *arg = (CScriptArray *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadArray( name->c_str(), arg, *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec2 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec2 *arg = (glm::vec2 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec2( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec3 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec3 *arg = (glm::vec3 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec3( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( LoadVec4 ) {
    const string_t *name = (const string_t *)pGeneric->GetArgObject( 0 );
    glm::vec4 *arg = (glm::vec4 *)pGeneric->GetArgObject( 1 );
    g_pArchiveHandler->LoadVec4( name->c_str(), glm::value_ptr( *arg ), *(int32_t *)pGeneric->GetArgAddress( 2 ) );
}

DEFINE_CALLBACK( ConstructVec3Generic ) {

}

DEFINE_CALLBACK( CopyConstructVec3Generic ) {

}

DEFINE_CALLBACK( ConsolePrint ) {
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    Con_Printf( "%s", msg->c_str() );
}

DEFINE_CALLBACK( GameError ) {
    const string_t *msg = (const string_t *)pGeneric->GetArgAddress( 0 );
    N_Error( ERR_DROP, "%s", msg->c_str() );
}

DEFINE_CALLBACK( RenderScene ) {
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
    refEntity_t refEntity;
    memset( &refEntity, 0, sizeof(refEntity) );
    refEntity.hShader = pGeneric->GetArgWord( 0 );
    VectorCopy( refEntity.origin, (float *)pGeneric->GetArgAddress( 1 ) );
    refEntity.flags = pGeneric->GetArgDWord( 2 );
}

DEFINE_CALLBACK( AddPolyToScene ) {
    CScriptArray *pPolyList = (CScriptArray *)pGeneric->GetArgAddress( 1 );
    re.AddPolyToScene( pGeneric->GetArgWord( 0 ), (const polyVert_t *)pPolyList->GetBuffer(), pPolyList->GetSize() );
}

DEFINE_CALLBACK( AddSpriteToScene ) {
    re.AddSpriteToScene( (const vec_t *)pGeneric->GetArgAddress( 0 ), pGeneric->GetArgWord( 1 ), pGeneric->GetArgWord( 2 ) );
}

DEFINE_CALLBACK( RegisterShader ) {
    pGeneric->SetReturnWord( re.RegisterShader( ( (const string_t *)pGeneric->GetArgAddress( 0 ) )->c_str() ) );
}

DEFINE_CALLBACK( RegisterSpriteSheet ) {
    const string_t *npath = (const string_t *)pGeneric->GetArgAddress( 0 );

    pGeneric->SetReturnWord( re.RegisterSpriteSheet( npath->c_str(), pGeneric->GetArgDWord( 1 ), pGeneric->GetArgDWord( 2 ),
        pGeneric->GetArgDWord( 3 ), pGeneric->GetArgDWord( 4 ) ) );
}

DEFINE_CALLBACK( ConstructPolyVert ) {
    memset( pGeneric->GetObject(), 0, sizeof(polyVert_t) );
}

DEFINE_CALLBACK( CopyConstructPolyVert ) {
    memcpy( pGeneric->GetObject(), pGeneric->GetArgObject( 0 ), sizeof(polyVert_t) );
}

DEFINE_CALLBACK( OpenFileRead ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenRead( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileWrite ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenWrite( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileAppend ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenAppend( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFileRW ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    *(nhandle_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenRW( fileName->c_str(), H_SGAME );
}

DEFINE_CALLBACK( OpenFile ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    fileHandle_t *hFile = (fileHandle_t *)pGeneric->GetArgAddress( 1 );
    fileMode_t mode = (fileMode_t )pGeneric->GetArgDWord( 2 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FOpenFile( fileName->c_str(), hFile, mode, H_SGAME );
}

DEFINE_CALLBACK( CloseFile ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    FS_VM_FClose( arg, H_SGAME );
}

DEFINE_CALLBACK( GetFileLength ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileLength( arg, H_SGAME );
}

DEFINE_CALLBACK( GetFilePosition ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileTell( arg, H_SGAME );
}

DEFINE_CALLBACK( FileSetPosition ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    uint32_t offset = pGeneric->GetArgDWord( 1 );
    uint32_t whence = pGeneric->GetArgDWord( 2 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_FileSeek( arg, offset, whence, H_SGAME );
}

DEFINE_CALLBACK( Write ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    const CScriptHandle *handle = (const CScriptHandle *)pGeneric->GetArgObject( 1 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_Write( handle->GetRef(), handle->GetType()->GetSize(), arg, H_SGAME );
}

DEFINE_CALLBACK( Read ) {
    fileHandle_t arg = *(fileHandle_t *)pGeneric->GetAddressOfArg( 0 );
    CScriptHandle *handle = (CScriptHandle *)pGeneric->GetArgObject( 1 );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = FS_VM_Read( handle->GetRef(), handle->GetType()->GetSize(), arg, H_SGAME );
}

DEFINE_CALLBACK( LoadFile ) {
    const string_t *fileName = (const string_t *)pGeneric->GetArgObject( 0 );
    CScriptArray *buffer = (CScriptArray *)pGeneric->GetArgObject( 1 );

    void *v;
    uint64_t length = FS_LoadFile( fileName->c_str(), &v );
    if ( !length || !v ) {
        Con_Printf( COLOR_RED "ERROR: failed to load file '%s' at vm request\n", fileName->c_str() );
        *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = 0;
        return;
    }

    buffer->Resize( length );
    memcpy( buffer->GetBuffer(), v, length );
    *(uint64_t *)pGeneric->GetAddressOfReturnLocation() = length;

    FS_FreeFile( v );
}

//
// script globals
//
static const int8_t script_S_COLOR_BLACK = S_COLOR_BLACK;
static const int8_t script_S_COLOR_RED = S_COLOR_RED;
static const int8_t script_S_COLOR_GREEN = S_COLOR_GREEN;
static const int8_t script_S_COLOR_YELLOW = S_COLOR_YELLOW;
static const int8_t script_S_COLOR_BLUE = S_COLOR_BLUE;
static const int8_t script_S_COLOR_CYAN = S_COLOR_CYAN;
static const int8_t script_S_COLOR_MAGENTA = S_COLOR_MAGENTA;
static const int8_t script_S_COLOR_WHITE = S_COLOR_WHITE;
static const int8_t script_S_COLOR_RESET = S_COLOR_RESET;

static const string_t script_COLOR_BLACK = COLOR_BLACK;
static const string_t script_COLOR_RED = COLOR_RED;
static const string_t script_COLOR_GREEN = COLOR_GREEN;
static const string_t script_COLOR_YELLOW = COLOR_YELLOW;
static const string_t script_COLOR_BLUE = COLOR_BLUE;
static const string_t script_COLOR_CYAN = COLOR_CYAN;
static const string_t script_COLOR_MAGENTA = COLOR_MAGENTA;
static const string_t script_COLOR_WHITE = COLOR_WHITE;
static const string_t script_COLOR_RESET = COLOR_RESET;

static const asQWORD script_MAX_TOKEN_CHARS = MAX_TOKEN_CHARS;
static const asQWORD script_MAX_STRING_CHARS = MAX_STRING_CHARS;
static const asQWORD script_MAX_STRING_TOKENS = MAX_STRING_TOKENS;
static const asQWORD script_MAX_INFO_KEY = MAX_INFO_KEY;
static const asQWORD script_MAX_INFO_STRING = MAX_INFO_STRING;
static const asQWORD script_MAX_INFO_VALUE = MAX_INFO_VALUE;
static const asQWORD script_MAX_CVAR_NAME = MAX_CVAR_NAME;
static const asQWORD script_MAX_CVAR_VALUE = MAX_CVAR_VALUE;
static const asQWORD script_MAX_EDIT_LINE = MAX_EDIT_LINE;
static const asQWORD script_MAX_NPATH = MAX_NPATH;
static const asQWORD script_MAX_OSPATH = MAX_OSPATH;
static const asQWORD script_MAX_VERTS_ON_POLY = MAX_VERTS_ON_POLY;
static const asQWORD script_MAX_UI_FONTS = MAX_UI_FONTS;

static const asQWORD script_MAX_INT8 = CHAR_MAX;
static const asQWORD script_MAX_INT16 = SHRT_MAX;
static const asQWORD script_MAX_INT32 = INT_MAX;
static const asQWORD script_MAX_INT64 = LONG_MAX;
static const asQWORD script_MAX_UINT8 = UCHAR_MAX;
static const asQWORD script_MAX_UINT16 = USHRT_MAX;
static const asQWORD script_MAX_UINT32 = UINT_MAX;
static const asQWORD script_MAX_UINT64 = ULONG_MAX;

static const asQWORD script_MIN_INT8 = CHAR_MIN;
static const asQWORD script_MIN_INT16 = SHRT_MIN;
static const asQWORD script_MIN_INT32 = INT_MIN;
static const asQWORD script_MIN_INT64 = LONG_MIN;

void ModuleLib_Register_Engine( void )
{
	SET_NAMESPACE( "TheNomad" );

    SET_NAMESPACE( "TheNomad::Constants" );
    { // Constants
        g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define S_COLOR_BLACK '%c'", S_COLOR_BLACK ) );
        g_pModuleLib->GetScriptBuilder()->DefineWord( va( "#define S_COLOR_RED '%c'", S_COLOR_RED ) );
        /*
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_BLACK", &script_S_COLOR_BLACK );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_RED", &script_S_COLOR_RED );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_GREEN", &script_S_COLOR_GREEN );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_YELLOW", &script_S_COLOR_YELLOW );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_CYAN", &script_S_COLOR_CYAN );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_MAGENTA", &script_S_COLOR_MAGENTA );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_WHITE", &script_S_COLOR_WHITE );
        REGISTER_GLOBAL_VAR( "const int8 TheNomad::Constants::S_COLOR_RESET", &script_S_COLOR_RESET );

        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_BLACK", &script_COLOR_BLACK );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_RED", &script_COLOR_RED );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_GREEN", &script_COLOR_GREEN );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_YELLOW", &script_COLOR_YELLOW );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_BLUE", &script_COLOR_BLUE );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_CYAN", &script_COLOR_CYAN );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_MAGENTA", &script_COLOR_MAGENTA );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_WHITE", &script_COLOR_WHITE );
        REGISTER_GLOBAL_VAR( "const string TheNomad::Constants::COLOR_RESET", &script_COLOR_RESET );

        static const asQWORD script_MAX_TOKEN_CHARS = MAX_TOKEN_CHARS;
        static const asQWORD script_MAX_STRING_CHARS = MAX_STRING_CHARS;
        static const asQWORD script_MAX_STRING_TOKENS = MAX_STRING_TOKENS;
        static const asQWORD script_MAX_INFO_KEY = MAX_INFO_KEY;
        static const asQWORD script_MAX_INFO_STRING = MAX_INFO_STRING;
        static const asQWORD script_MAX_INFO_VALUE = MAX_INFO_VALUE;
        static const asQWORD script_MAX_CVAR_NAME = MAX_CVAR_NAME;
        static const asQWORD script_MAX_CVAR_VALUE = MAX_CVAR_VALUE;
        static const asQWORD script_MAX_EDIT_LINE = MAX_EDIT_LINE;
        static const asQWORD script_MAX_NPATH = MAX_NPATH;
        static const asQWORD script_MAX_OSPATH = MAX_OSPATH;
        static const asQWORD script_MAX_VERTS_ON_POLY = MAX_VERTS_ON_POLY;
        static const asQWORD script_MAX_UI_FONTS = MAX_UI_FONTS;

        static const asQWORD script_MAX_INT8 = CHAR_MAX;
        static const asQWORD script_MAX_INT16 = SHRT_MAX;
        static const asQWORD script_MAX_INT32 = INT_MAX;
        static const asQWORD script_MAX_INT64 = LONG_MAX;
        static const asQWORD script_MAX_UINT8 = UCHAR_MAX;
        static const asQWORD script_MAX_UINT16 = USHRT_MAX;
        static const asQWORD script_MAX_UINT32 = UINT_MAX;
        static const asQWORD script_MAX_UINT64 = ULONG_MAX;

        static const asQWORD script_MIN_INT8 = CHAR_MIN;
        static const asQWORD script_MIN_INT16 = SHRT_MIN;
        static const asQWORD script_MIN_INT32 = INT_MIN;
        static const asQWORD script_MIN_INT64 = LONG_MIN;
        static const asQWORD script_MIN_UINT8 = UCHAR_MIN;
        static const asQWORD script_MIN_UINT16 = USHRT_MIN;
        static const asQWORD script_MIN_UINT32 = UINT_MIN;
        static const asQWORD script_MIN_UINT64 = ULONG_MIN;
        */
    }
	
	SET_NAMESPACE( "TheNomad::Engine" );
	{ // Engine
        
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarRegister( const string&, string&, uint, int64&, float&, int&, int& )", CvarRegisterGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarSet( const string&, const string& )", CvarSetValueGeneric );
        REGISTER_GLOBAL_FUNCTION( "void TheNomad::CvarUpdate( const string&, string&, int64&, float&, int&, const int )", CvarUpdateGeneric );
		
		{ // SoundSystem
			SET_NAMESPACE( "TheNomad::Engine::SoundSystem" );
			
			// could this be a class, YES, but I won't force it on the modder
			
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::PlaySfx( int )", Snd_PlaySfx );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::SetLoopingTrack( int )", Snd_SetLoopingTrack );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::SoundSystem::ClearLoopingTrack()", Snd_ClearLoopingTrack );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterSfx( const string& )", Snd_RegisterSfx );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::SoundSystem::RegisterTrack( const string& )", Snd_RegisterTrack );

            RESET_NAMESPACE();
		}
		{ // FileSystem
			SET_NAMESPACE( "TheNomad::Engine::FileSystem" );
			
			// could this be a class, YES, but I won't force it on the modder
			
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileRead( const string& )", OpenFileRead );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileWrite( const string& )", OpenFileWrite );
			REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::FileSystem::OpenFileAppend( const string& )", OpenFileAppend );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::OpenFile( const string&, int, int& )", OpenFile );
			REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::FileSystem::CloseFile( int )", CloseFile );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetLength( int )", GetFileLength );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::GetPosition( int )", GetFilePosition );
            REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::SetPosition( int, uint, uint )", FileSetPosition );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::Write( const ref@, uint64, int )", Write );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::Read( ref@, uint64, int )", Read );
			REGISTER_GLOBAL_FUNCTION( "uint64 TheNomad::Engine::FileSystem::LoadFile( const string&, array<int8>& )", LoadFile );

            RESET_NAMESPACE();
		}
        { // Renderer
            SET_NAMESPACE( "TheNomad::Engine::Renderer" );

//            REGISTER_OBJECT_TYPE( "PolyVert", polyVert_t, asOBJ_GC | asOBJ_REF );
//            REGISTER_OBJECT_BEHAVIOUR( "TheNomad::Engine::Renderer::PolyVert", asBEHAVE_CONSTRUCT, "void f()", ConstructPolyVert );

            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_RenderScene( uint, uint, uint, uint, uint, uint )", RenderScene );
//            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddEntityToScene( int, vec3, uint,  )", AddEntityToScene );
//            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddPolyToScene( int, const array<PolyVert>@ )", AddPolyToScene );
            REGISTER_GLOBAL_FUNCTION( "void TheNomad::Engine::Renderer::RE_AddSpriteToScene( float, float, float, int, int )", AddSpriteToScene );
            REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RE_RegisterShader( const string& )", RegisterShader );
            REGISTER_GLOBAL_FUNCTION( "int TheNomad::Engine::Renderer::RE_RegisterSprite( const string&, uint, uint, uint, uint )", RegisterSpriteSheet );

            RESET_NAMESPACE();
        }
	}
    
    SET_NAMESPACE( "TheNomad" );
	SET_NAMESPACE( "TheNomad::GameSystem" );
	{
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
    }

    RESET_NAMESPACE();
	
	//
	// misc & global funcdefs
	//
	
	REGISTER_GLOBAL_FUNCTION( "void ConsolePrint( const string& )", ConsolePrint );
	REGISTER_GLOBAL_FUNCTION( "void GameError( const string& )", GameError );
    REGISTER_GLOBAL_FUNCTION( "void Assert( bool bValue )", ModuleAssertion );
}
